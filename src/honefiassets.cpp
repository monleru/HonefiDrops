#include <honefiassets.hpp>

  ACTION honefiassets::createdrop(int dropnum, int changeprice,int changepricetime, string format, name username, name collection, name shemas, uint32_t templates, asset price, int supply, uint64_t dropstart, uint64_t dropend, string img,string drop_name, string description){
  require_auth(username);
  check ( format == "balancer" || format == "dutch", "Invalid forman");
  atomicassets::collections_t  collections  = atomicassets::collections_t(atomicassets::ATOMICASSETS_ACCOUNT, atomicassets::ATOMICASSETS_ACCOUNT.value);
  auto collection_itr = collections.require_find(collection.value);
  check ( collection_itr->author == username, "Error auth collection");
  // drop num table find
  auto now = current_time_point().sec_since_epoch();
  check ( now < dropstart, "The beggining connot be earlier than the current time" );
  auto itr_dropnum = config_table.find(get_self().value);
  // Create drop
  drop_table.emplace(username, [&](auto& row) {
    row.dropnum = itr_dropnum->dropnum + 1;
    row.format = format;
    row.username = username;
    row.collection = collection;
    row.shemas = shemas;
    row.templates = templates;
    row.startprice = asset{price.amount, price.symbol};
    row.price = asset{price.amount, price.symbol};
    row.changeprice = changeprice;
    row.changepricetime = changepricetime;
    row.maxsupply = supply;
    row.dropstart = dropstart;
    row.dropend = dropend;
    row.lastbuy = dropstart;
    row.img = img;
    row.drop_name = drop_name;
    row.description = description;
  });
  // drop num number + 1
  config_table.modify(itr_dropnum, username, [&](auto& row) {
  row.dropnum += 1;
  });
}
// registration new user
ACTION honefiassets::newuser(name username){
  require_auth(username);
  auto itr = users_config.find(username.value);
  check ( itr == users_config.end(), "You already registr");
  users_config.emplace(username, [&](auto& row) {
    row.username = username;
    row.slip = 2.5;
  });
}
// slippage
ACTION honefiassets::setslip(name username, float slip){
  require_auth(username);
  check ( slip <= 50, "Maximum slip 50%");
  auto itr = users_config.find(username.value);
  check ( itr != users_config.end(), "You need registr");
  users_config.modify(itr, username, [&](auto& row) {
    row.slip = slip;
  });
}

void honefiassets::token_transfer(name from,name to, asset quantity, string memo){
  // auth user
  require_auth(from);
  // check transfer wallet
  if (to != get_self() || from == get_self()) return;
  in_contract_claim(from, memo, quantity);
  require_recipient(from);
}
//User ram

// ...
ACTION honefiassets::dropremove(int drop_id){
  auto itr_drop = drop_table.find(drop_id);
  require_auth(itr_drop->username);
  itr_drop = drop_table.erase(itr_drop);
}
//set config
ACTION honefiassets::config(){
  require_auth(get_self());
  config_table.emplace(get_self(), [&](auto& row) {
    row.username = get_self();
    row.dropnum = 0;
  });
}
//mint
void honefiassets::in_contract_claim(name username, string memo, asset quantity){
  // memo(int)
  int drop_id = stoi(memo);
  //find drop table
  auto itr = drop_table.find(drop_id);
  // check drop in drop table
  check ( itr != drop_table.end(), "Invalid drop");
  // time
  uint64_t now = current_time_point().sec_since_epoch();
  // check drop start
  check( now >= itr->dropstart,"Drop didn't start");
  //check end drop
  if ( itr->dropend != 0 ) {
    check ( now <= itr->dropend, "Drop ended");
  }
  check ( itr->supply < itr->maxsupply, "The nfts is over");
  if ( itr->format == "balancer") {
    // Balance pool formula
    float sss = 100;
    float pricec = 1 - itr->changeprice/sss;
    pricec = pow(pricec, floor((now - itr->lastbuy)/itr->changepricetime));
    float price_now = itr->price.amount*pricec;
    // check token quality
    if (quantity.amount > price_now){
      // mint function
      // mint data
      vector<name> data;
      //mint nft
      action{
        permission_level{get_self(), "active"_n},
        "atomicassets"_n,
        "mintasset"_n,
        std::make_tuple(get_self(),itr->collection, itr->shemas, itr->templates, username, data, data, data)
      }.send();
      // Return slippage
      if ( quantity.amount > price_now){
        quantity.amount = quantity.amount - price_now;
        in_contract_transfer(username, quantity);
      }
      // new price and new lastbuy time point
      drop_table.modify(itr, get_self(), [&](auto& row) {
        row.price.amount = price_now + (price_now / sss)*itr->changeprice;
        row.lastbuy = current_time_point().sec_since_epoch();
        row.supply += 1;
      });
      // send wax to creator drop
      quantity.amount = price_now;
      in_contract_transfer(itr->username, quantity); 
    }
    else check(false, "You don't have enough founds");
  }
  else if ( itr->format == "datch" ) {
    // Calculating the price
    float checkprice = (now - itr->dropstart)/itr->changepricetime;
    int price_now = floor(itr->startprice.amount - (itr->startprice.amount/100*floor(checkprice))*itr->changeprice);
    asset price__now = itr->startprice;
    // check the price, if the price os less than 0, we leave it at 0.
    if ( itr->startprice.amount - price_now > 0 ) {
      price__now.amount = itr->startprice.amount - price_now;
    }
    else {
      price__now.amount = 0;
    }
    // check payment founds
    check ( quantity.amount == price__now.amount , "You don't have enough founds");
    // mint data
    vector<name> data;
    //mint nft
    action{
      permission_level{get_self(), "active"_n},
      "atomicassets"_n,
      "mintasset"_n,
      std::make_tuple(get_self(),itr->collection, itr->shemas, itr->templates, username, data, data, data)
    }.send();
    // change number of purchases
    drop_table.modify(itr, get_self(), [&](auto& row) {
      row.supply += 1;
    });
    // sending funds to the creator of the drop
    in_contract_transfer(itr->username, quantity); 


  }
}
// transfer tokens
void honefiassets::in_contract_transfer(name username, asset quantity){
  string memo;
  action{
    permission_level{get_self(), "active"_n},
    "eosio.token"_n,
    "transfer"_n,
    std::make_tuple(get_self(),username, quantity, memo)
  }.send();
}
//set drop time
ACTION honefiassets::setdroptime(int drop_id, uint64_t dropstart, uint64_t dropend){
  auto itr_drop = drop_table.find(drop_id);
  require_auth(itr_drop->username);
  drop_table.modify(itr_drop, get_self(), [&](auto& row) {
    row.dropstart = dropstart;
    row.dropend = dropend;
  });
}
//set price
ACTION honefiassets::setprice(int drop_id, asset price){
  auto itr_drop = drop_table.find(drop_id);
  require_auth(itr_drop->username);
  drop_table.modify(itr_drop, get_self(), [&](auto& row) {
    row.startprice = asset{price.amount, price.symbol};
    row.price = asset{price.amount, price.symbol};
  });
}
// set drop info
ACTION honefiassets::setinfo(int drop_id, string img, string drop_name, string description){
  auto itr_drop = drop_table.find(drop_id);
  require_auth(itr_drop->username);
  drop_table.modify(itr_drop, get_self(), [&](auto& row) {
    row.img = img;
    row.drop_name = drop_name;
    row. description = description;
  });
}
// set drop supply
ACTION honefiassets::setsupply( int drop_id, int supply){
  auto itr_drop = drop_table.find(drop_id);
  require_auth(itr_drop->username);
  drop_table.modify(itr_drop, get_self(), [&](auto& row) {
    row.maxsupply = supply;
  });
}
// set change prices
ACTION honefiassets::setchangepr( int drop_id, int changeprice,int changepricetime){
  auto itr_drop = drop_table.find(drop_id);
  require_auth(itr_drop->username);
  drop_table.modify(itr_drop, get_self(), [&](auto& row) {
    row.changeprice = changeprice;
    row.changepricetime = changepricetime;
  });
}
