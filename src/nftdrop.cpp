#include <honefiassets.hpp>

// Create nft Drop
ACTION honefiassets::cnftdrop(
      string format,
      name username,
      name collection,
      name shemas,
      uint32_t templates,
      asset price,
      int supply,
      uint64_t dropstart,
      uint64_t dropend,
      int changeprice,
      int changepricetime,
      string img,
      string drop_name,
      string description){
  require_auth(username);
  check ( format == "balance" || format == "datch", "Invalid forman");
  check ( dropend <= 480 && dropend > 5, "Invalid dropend time");
  if ( format == "datch" ) {
    check( changeprice > 0 && changeprice <= 10, "Invalid 'changeprice' options");
    check( changepricetime >= 30, "Invalid 'changepricetime' options");
  }
  atomicassets::collections_t  collections  = atomicassets::collections_t(atomicassets::ATOMICASSETS_ACCOUNT, atomicassets::ATOMICASSETS_ACCOUNT.value);
  auto collection_itr = collections.require_find(collection.value);
  check ( collection_itr->author == username, "Error auth collection");
  // drop num table find
  auto now = current_time_point().sec_since_epoch();
  check ( now < dropstart, "The beggining connot be earlier than the current time" );
  // Create drop
  auto itr_dropnum = config_table.find(get_self().value);
  drop_table.emplace(username, [&](auto& row) {
    row.dropnum = itr_dropnum->dropnum + 1;
    row.format = format;
    row.username = username;
    row.collection = collection;
    row.shemas = shemas;
    row.templates = templates;
    row.startprice = asset{price.amount, price.symbol};
    row.price = asset{price.amount, price.symbol};
    row.maxsupply = supply;
    row.dropstart = dropstart;
    row.dropend = dropstart + dropend*60;
    row.lastbuy = dropstart;
    row.changeprice = changeprice;
    row.changepricetime = changepricetime;
    row.img = img;
    row.drop_name = drop_name;
    row.description = description;
  });
  // drop num number + 1
  config_table.modify(itr_dropnum, username, [&](auto& row) {
  row.dropnum += 1;
  });
}



//// claimdrop
ACTION honefiassets::claimdrop ( name claimer, int drop_id, int claim_amount ){
  require_auth(claimer);
  auto itr_user = users_config.find(claimer.value);
  check ( itr_user != users_config.end(), "Error");
  asset quantity = itr_user->balance;
  //find drop table
  auto itr = drop_table.find(drop_id);
  // check drop in drop table
  // check ( itr != drop_table.end(), "Invalid drop");


  // check ram balance
  auto itr_rambalance = _rambalance.find(itr->collection.value);
  check ( itr_rambalance->bytes >= 151, "The collection has no ram");
  check ( itr_rambalance != _rambalance.end(), "The collection has no ram");
  _rambalance.modify(itr_rambalance, claimer, [&](auto& row) {
    row.bytes -= 151;
  });

  //get Time
  uint64_t now = current_time_point().sec_since_epoch();
  // check drop start
  check( now >= itr->dropstart,"Drop didn't start");
  //check end drop
  check ( now <= itr->dropend, "Drop ended");
  // check Supply
  check ( itr->supply < itr->maxsupply, "The nfts is over");
  auto config_ = config_table.find(get_self().value);
  if ( itr->format == "balance") {
    // Balance pool formula
    float sss = 100;
    float pricec = 1 - 1/sss;
    auto num_ = floor((now - itr->lastbuy)/30);
    float price_now;
    if (num_ > 1000) {
        price_now = config_->min_price.amount;
    } else {
        pricec = pow(pricec, num_);
        price_now = itr->price.amount*pricec;
        if ( price_now < config_->min_price.amount) {
            price_now = config_->min_price.amount;
        } 
    }
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
        std::make_tuple(get_self(),itr->collection, itr->shemas, itr->templates, claimer, data, data, data)
      }.send();
      // Return slippage
      users_config.modify(itr_user, claimer, [&](auto& row) {
        row.balance.amount -= price_now;
      });
      // new price and new lastbuy time point
      int buy__ = floor(itr->maxsupply/100);
      if (buy__ < 1 ) { buy__ = 1; };
      if ( (itr->buy + 1) >= buy__ ) {
        drop_table.modify(itr, get_self(), [&](auto& row) {
            row.price.amount = price_now + (price_now / sss);
            row.lastbuy = current_time_point().sec_since_epoch();
            row.supply += 1;
            row.buy = 0;
        });
      } else {
        drop_table.modify(itr, get_self(), [&](auto& row) {
            row.supply += 1;
            row.buy += 1;
        });
      }
      // send wax to creator drop
      quantity.amount = price_now;
      in_contract_transfer(itr->username, quantity); 
    }
    else check(false, "You don't have enough founds");
  }
  else if ( itr->format == "datch" ) {
    // Calculating the price
    int checkprice = floor((now - itr->dropstart)/itr->changepricetime);
    asset price_now = itr->startprice;
    price_now.amount = itr->startprice.amount - (itr->startprice.amount/100*checkprice)*itr->changeprice;
    asset price__now = itr->startprice;
    // check the price, if the price os less than 0, we leave it at 0.
    if ( price_now.amount > config_->min_price.amount ) {
      price__now.amount = price_now.amount;
    }
    else {
      price__now.amount = config_->min_price.amount;
    }
    // check payment founds
    check ( quantity.amount >= price__now.amount , "You don't have enough founds");
    // mint data
    vector<name> data;
    //mint nft
    action{
      permission_level{get_self(), "active"_n},
      "atomicassets"_n,
      "mintasset"_n,
      std::make_tuple(get_self(),itr->collection, itr->shemas, itr->templates, claimer, data, data, data)
    }.send();
    // change number of purchases
    drop_table.modify(itr, get_self(), [&](auto& row) {
      row.supply += 1;
    });
    users_config.modify(itr_user, claimer, [&](auto& row) {
      row.balance.amount -= price__now.amount;
    });
    // sending funds to the creator of the drop
    in_contract_transfer(itr->username, price__now);
  }
}