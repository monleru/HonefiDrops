## 👋 Honefi proposal on WAX LABS

This repository contains information regarding the development of the [HoneFi platform](https://honefi.app), including a roadmap, which is specified in the [proposal](https://labs.wax.io/proposals/81), as well as reports on deliverables. The reports are published with a description of the work done, as well as technical details if the deliverables contain development of any functionality.

For convenient activity monitoring, we have made a navigation of all currently started and completed tasks, which is shown below.

## 📜 List of Deliverables

1. [Auction Platform]().
2. [LaunchPool Platform]().
3. [Backend Development]().
4. [Final Stage]().


###  Auction Platform

The Auction Platform is a tool for two types of auctions (balance and Dutch) that works with both NFTs and tokens. The development is divided into several stages:
- [x] Creating UI/UX in a modern style
- [x] Frontend development for NFT's auctions and tokens sections
- [x] Development of smart contracts for each type of auction (NFT and tokens)
- [x] Developing an auction dashboard

#### UI/UX

>List of auctions and filter

<img src="https://i.postimg.cc/HVCK983d/Screenshot-2022-12-04-064238.png" height="150"/> <img src="https://i.postimg.cc/YjFZFz6d/Screenshot-2022-12-04-064340.png" height="150"/>

>Auction page

<img src="https://i.postimg.cc/ZBNmCtYc/Screenshot-2022-12-04-064830.png" height="150"/> <img src="https://i.postimg.cc/DWHFvyh0/Screenshot-2022-12-04-064853.png" height="150"/> <img src="https://i.postimg.cc/21Hrt7X4/Screenshot-2022-12-04-065036.png" height="150"/>

>Auction Dashboard

<img src="https://i.postimg.cc/N9K6XDZb/Screenshot-2022-12-04-065324.png" height="150"/> <img src="https://i.postimg.cc/C1YcrVjH/Screenshot-2022-12-04-065359.png" height="150"/> <img src="https://i.postimg.cc/7LSBNLTZ/Screenshot-2022-12-04-065418.png" height="150"/> <img src="https://i.postimg.cc/13n7fBm6/Screenshot-2022-12-04-065438.png" height="150"/> <img src="https://i.postimg.cc/BnrYDFbz/Screenshot-2022-12-04-065458.png" height="150"/> <img src="https://i.postimg.cc/8Pnnp9SN/Screenshot-2022-12-04-065528.png" height="150"/>

#### Frontend development for NFT's auctions and tokens sections

We have developed Frontend for all the necessary functionality related to creating and participating in auctions. You can use https://honefi.app to test the full functionality. The main tasks performed:
- Connecting authentication for WAX cloud wallet and anchor
- Auction creation page - https://honefi.app/create
- Current NFT auctions page (and filter) - https://honefi.app/drops
- Current Tokens auctions page (and filter) - https://honefi.app/tokens
- Auction pages containing purchase, settings, auction information and all social media

#### Development of smart contracts for each type of auction (NFT and tokens)

HoneFi Main Smart Contract - **honefidropsx

The source code of the smart contract is uploaded to this repository and can be viewed [here](links)

Technical details:

>**buyram** - topping up the RAM balance for a particular collection.

| Field Name  | Type  | Description |
|---|---|---|
| username  | name | WAX Wallet |
| collection  | name  | Collection Name |
| quant | asset | Number of Tokens |

>**claimbalance** - getting a balance.

| Field Name  | Type  | Description |
|---|---|---|
| username  | name | WAX Wallet |

>**claimdrop** - buying Assets at NFT's Auction.

| Field Name  | Type  | Description |
|---|---|---|
| claimer  | name | WAX Wallet |
| drop_id  | int32  | Drop ID |
| claim_amount | int32 | Number of Assets to Buy |

>**claimtdrop** - buying Assets at Tokens Auction.

| Field Name  | Type  | Description |
|---|---|---|
| claimer  | name | WAX Wallet |
| drop_id  | int32  | Drop ID |
| amount | asset | Number of Assets to Buy |

>**cnftdrop** - creation of the NFT's auction.

| Field Name  | Type  | Description |
|---|---|---|
| format  | string | Auction format (Dutch/Balance) |
| username  | name  | WAX Wallet |
| collection | name | Collection Name |
| shemas | name | Shemas Name |
| templates | uint32 | Templates ID |
| price | asset | Start Price |
| supply | int32 | Supply for Auction |
| dropstart | uint64 | UNIX time (start auction) |
| dropend | uint64 | UNIX time (duration) |
| changeprice | int32 | Percent to change |
| changepricetime | int32 | Time to change |
| img | string | IPFS hash |
| drop_name | string | Drop Name |
| description | string | Drop Description |

>**config** - minimum price mention.

| Field Name  | Type  | Description |
|---|---|---|
| min_price  | asset | Minimum Price |

>**ctokendrop** - creation of the Tokens auction.

| Field Name  | Type  | Description |
|---|---|---|
| format  | string | Auction format (Dutch/Balance) |
| username  | name  | WAX Wallet |
| collection | name | Collection Name |
| tokenticker | string | Token Ticker |
| contract_adress | name | WAX Adress |
| maxbuy_tx | int32 | Max Assets per transaction |
| price | asset | Start Price |
| supply | asset | Supply for Auction |
| dropstart | uint64 | UNIX time (start auction) |
| dropend | uint64 | UNIX time (duration) |
| changeprice | int32 | Percent to change |
| changepricetime | int32 | Time to change |
| img | string | IPFS hash |
| drop_name | string | Drop Name |
| description | string | Drop Description |

>**dropremove** - Drop by ID deletion.

| Field Name  | Type  | Description |
|---|---|---|
| drop_id  | int32 | Drop ID |

**We have prepared a video on creating an auction and branding assets - links.

#### Developing an auction dashboard

To create auctions, we made a management page, which is available after connecting the wallet to the site - https://honefi.app/create

Creation takes place in several stages:
- Select the desired collection and add auth to the collection (approve)
- Go to collection management (the "Manage" button) and get available functions, which include - NFT drop page, Token drop page, top up RAM balance for the collection
- When you go to one of the drop management pages, you can either create a new dropbox or go to an existing one.
- To create a new drop, you need to fill in all the fields on the page - the name, description, price changes in % and minutes, add a cover and specify the start time of the auction, as well as its duration
- In the next step, you can check how your drop will look after creation. If everything is filled in correctly, you can create the drop