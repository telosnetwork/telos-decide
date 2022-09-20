# Decide Content Metadata Specification (DCMS v2)
## An Update to Extend Telos Decide Functionality

### Intro

As applications continue to develop upon the currently Telos Decide functionality, it becomes necessary to extend the contract's functionality to improve the developer and user experience. Given the resources needed to modify the current smart contract, a simpler approach is needed that could be achieved with the code base as-is. This approach requires the work to be done on the application layer, and the creation of a standard to be adopted by application developers.

Specifically, this new specification proposes to add the following functionality through a JSON object in the existing `content` field:

1.  Since this metadata will be residing within the `content` columns of the tables, this means that the hyperlinks to externally-hosted content files (typically PDFs, images, or Word documents) will be moved to a new `contentUrls` field. **Critically, this means that any dapp reading the `content` field will have to inspect its value to ascertain how the author of the Works proposal or regular ballot intends for it to be used (more on that later)**.
2.  An `image` field for Works proposals or regular ballots that apps could display to give them context
3.  A `displayText` field for Works proposals milestones and regular ballot options (excluding elections and leaderboards) that gives them a human-readable **title**, in lieu of the 12-character key, that can extend past 12 characters. For example, a poll could shown an option for a ballot on where to place an office location as `Jackson, Mississippi` instead of `jacksonmissi`. Additionally, the first milestone for a Works proposal looking for funding for a new park in Hong Kong could have its first milestone displayed as `Environmental Impact Study` rather than a milestone ID of `1` or `miss.option1` (which are not very informative)
4.  An `imageUrls` field for each Works milestone and regular ballot option. These images will help give end-users context for milestones and ballot options, and help create a more visually-appealing dapp. For example, a poll on favorite ice cream flavors could display an image (or images) of each flavor, or the aforementioned Hong Kong park example could show an image of the proposed park location for its first milestone ("Environmental Impact Study").
5.  A `description` field for each Works milestone and regular ballot option. In addition to images and human-readable titles (`displayText`), each milestone or option could have an associated long-form description that could give users a more comprehensive explanation of what they entail. While this field is designed to allow for more text than the `displayText` field, the number of characters allowed by dapps should still be limited since this field is likely to be displayed next to each option in a **list**. An end-user should be able to quickly read through each milestone / option's descriptions to get an overview of how each item compares to the others in the list. More thorough descriptions should be placed within the proposal / ballot's `contentUrls` field.
6.  A `version` field to identify the specification version (currently `v2`) to help dapps properly parse the metadata.

**Note**: since JSON objects are extensible, dapps can also enter additional metadata into the JSON object although other dapps may not use said information. An example could be a `producedBy` field that includes the information about the dapp that was used to help author the proposal / ballot.

### Parsing the `content` Field

Since users are currently using the `content` field to enter a URL, and dapps are attempting to display the value as a hyperlink, this means that dapps will need to implement code that conditionally displays the information based on the value type. The code would look something like this:

```
let contentUrls, imageUrls, optionData
try {
  const parsedContent  = JSON.parse(content)
  contentUrls = parsedContent.contentUrls
  imageUrls = parsedContent.imageUrls
  optionData = parsedContent.optionData
} catch (err) {
  try {
    contentUrl = new URL(content)
  } catch (err) {
    // do something else, likely `Qm...` hash for IPFS / dStor
  }
}
```

Or, more succinctly (using destructuring)...

```
let contentUrls, imageUrls, optionData
try {
  ({ contentUrls, imageUrls, optionData }  = JSON.parse(content))
} catch (err) {
  try {
    contentUrl = new URL(content)
  } catch (err) {
    // do something else, likely `Qm...` hash for IPFS / dStor
  }
}
```

or

```
let content
try {
  content = JSON.parse(content))
} catch (err) {
  console.log('content not parsible')
}

console.log('first imageUrls item is: ', content?.imageUrls?[0])
```

Additionally, since each property is optional, the dapp will have to decide how to display the metadata conditionally based on whether or not each field exists (ie an option with an `imageUrls` value but no `displayText` value, etc)

### Proposed `content` Field Structure

The JSON object structure will look something like this for a Works proposal:

```
{
  "version": "DCMSv2",
  "imageUrls": array of strings, // URLs or hashes for images associated with Works proposal
  "contentUrls": array of strings // URLs or hashes for documents associated with Works proposal
  "milestonesData":[
    {
      "displayText": string, // relatively short human-readable title for first milestone
      "description": string, // medium-length description for first milestone
      "imageUrls": array of strings // URLs or hashes for imagees associated with first milestone
    },{
      "displayText": string, // relatively short human-readable title for second milestone
      "description": string, // medium-length description for second milestone
      "imageUrls": array of strings // URLs or hashes for image associated with second milestone
    }
  ]
}
```

Likewise, the JSON object structure for a regular `telos.decide` ballot would look something like this:

```
{
  "version": "DCMSv2",
  "imageUrls": array of strings, // URLs or hashes for images associated with Works proposal
  "contentUrls": array of strings // URLs or hashes for documents associated with Works proposal
  "optionData": {
    "option1...": { // key (`option1...`) must be same as the actual option key in `options` field of the `ballots` table
      "displayText": string, // relatively short human-readable title for first option
      "description": string, // medium-length description for first option
      "imageUrls": array of strings // URLs or hashes for images associated with first option
    },
    "option2...": { // key (`option2...`) must be same as the actual option key in `options` field of the `ballots` 
      "displayText": string, // relatively short human-readable title for second option
      "description": string, // medium-length description for second option
      "imageUrls": array of strings // URLs or hashes for images associated with second option
    }
  }
}
```

### Extra Consideration

Since adding metadata to the `content` field will take up memory, we recommend that dapps helping to author proposals and ballots consider omitting any fields that are not being used. For example, a milestone's `imageUrls` field should be omitted if the user has not specified an image for that milestone.

### Discussion and Contact Info

Any questions or discussion regarding this specification can be had by contact the specification's author (Kylan Hurt, kylan.hurt@gmail.com) or using the Telos Governance Telegram Channel at the following URL:  
https://t.me/telosgovernance