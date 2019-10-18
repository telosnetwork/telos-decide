#include <trail.hpp>

//======================== payroll actions ========================

ACTION trail::addfunds(name from, symbol treasury_symbol, name payroll_name, asset quantity) {
    
    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(treasury_symbol.code().raw(), "treasury not found");

    //validate
    check(quantity.symbol == TLOS_SYM, "only TLOS allowed in payrolls");

    //open payrolls table, get payroll
    payrolls_table payrolls(get_self(), treasury_symbol.code().raw());
    auto& pr = payrolls.get(payroll_name.value, "payroll not found");

    //charge quantity to account
    require_fee(from, quantity);

    //debit quantity to payroll funds
    payrolls.modify(pr, same_payer, [&](auto& col) {
        col.payroll_funds += quantity;
    });
}

ACTION trail::editpayrate(name payroll_name, symbol treasury_symbol, uint32_t period_length, asset per_period) {
    
    //open payrolls table, get payroll
    payrolls_table payrolls(get_self(), treasury_symbol.code().raw());
    auto& pr = payrolls.get(payroll_name.value, "payroll not found");

    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(treasury_symbol.code().raw(), "treasury not found");

    //authenticate
    require_auth(trs.manager);

    //validate
    check(period_length > 0, "period length must be greater than 0");
    check(per_period.amount > 0, "per period pay must be greater than 0");

    //update pay rate
    payrolls.modify(pr, same_payer, [&](auto& col) {
        col.period_length = period_length;
        col.per_period = per_period;
    });

}