#include <honefiassets.hpp>

ACTION honefiassets::ctokendrop(
  name username, 
  name collection,
  string tokenticker,
  int token_decimals,
  name contract_adress,
  int maxbuy_tx,
  string format,
  asset price, 
  asset supply, 
  int changeprice,
  int changepricetime,
  uint64_t dropstart, 
  uint64_t dropend, 
  string img,
  string drop_name, 
  string description){
  require_auth(username);
  check ( format == "balance" || format == "datch", "Invalid forman");
    auto now = current_time_point().sec_since_epoch();
  check ( now < dropstart, "The beggining connot be earlier than the current time" );
  // Create drop
  auto itr_dropnum = config_table.find(get_self().value);
  _tokendrop.emplace(username, [&](auto& row) {
    row.dropnum = itr_dropnum->dropnum + 1;
    row.format = format;
    row.username = username;
    row.collection = collection;
    row.tokenticker = tokenticker;
    row.token_decimals = token_decimals;
    row.contract_adress = contract_adress;
    row.changeprice = changeprice;
    row.changepricetime = changepricetime;
    row.maxbuy_tx = maxbuy_tx;
    row.startprice = asset{price.amount, price.symbol};
    row.price = asset{price.amount, price.symbol};
    row.maxsupply = supply;
    row.supply = asset{0, supply.symbol};
    row.dropstart = dropstart;
    row.dropend = dropstart + dropend*60;
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




//// claimdrop
ACTION honefiassets::claimtdrop ( name claimer, int drop_id, int amount){
  require_auth(claimer);
  auto itr_user = users_config.find(claimer.value);
  check ( itr_user != users_config.end(), "Error");
  asset quantity = itr_user->balance;
  //find drop table
  auto itr = _tokendrop.find(drop_id);
  // check drop in drop table
  check ( itr != _tokendrop.end(), "Invalid drop");
  //get Time
  uint64_t now = current_time_point().sec_since_epoch();
  // check drop start
  check( now >= itr->dropstart,"Drop didn't start");
  //check end drop
  check ( now <= itr->dropend, "Drop ended");
  // check Supply
  int64_t amount_ = amount*pow(10,itr->token_decimals);
  check ( amount <= itr->maxbuy_tx, "error mith max supply for buy" );
  check ( amount_ < (itr->maxsupply.amount - itr->supply.amount), "The tokenks is over");
  auto config_ = config_table.find(get_self().value);
  if ( itr->format == "balance") {
    // Balance pool formula
    float sss = 100;
    float pricec = 1 - 1/sss;
    auto num_ = floor((now - itr->lastbuy)/30);
    asset price_now = asset{0, itr->price.symbol};
    if (num_ > 1000) {
        price_now.amount = config_->min_price.amount*amount;
    } else {
        pricec = pow(pricec, num_);
        price_now.amount = (itr->price.amount*pricec)*amount;
        if ( price_now.amount < config_->min_price.amount) {
            price_now.amount = config_->min_price.amount*amount;
        } 
    }
    // check token quality
    if (quantity.amount >= price_now.amount){
      // mint function
      action(
          permission_level{CONTRACTN, name("active")},
          itr->contract_adress,
          "transfer"_n,
          make_tuple(get_self(), claimer, asset{amount_, itr->supply.symbol}, string("Buy tokens")))
        .send();
      // Return slippage
      users_config.modify(itr_user, claimer, [&](auto& row) {
        row.balance.amount -= price_now.amount;
      });
      // new price and new lastbuy time point
      int buy__ = itr->maxsupply.amount/pow(10,itr->token_decimals + 2);
      if (buy__ < 1 ) { buy__ = 1; };
      if ( (itr->buy + amount) >= buy__ ) {
        _tokendrop.modify(itr, get_self(), [&](auto& row) {
            row.price.amount = price_now.amount/amount + ((price_now.amount/amount) / sss);
            row.lastbuy = current_time_point().sec_since_epoch();
            row.supply.amount += amount_;
            row.buy = 0;
        });
      } else {
        _tokendrop.modify(itr, get_self(), [&](auto& row) {
            row.supply.amount += amount_;
            row.buy += amount;
        });
      }
      // send wax to creator drop
      quantity.amount = price_now.amount;
      in_contract_transfer(itr->username, quantity);
    }
    else check(false, "You don't have enough founds");
  }
  else if ( itr->format == "datch" ) {
    // Calculating the price
    int checkprice = floor((now - itr->dropstart)/itr->changepricetime);
    asset price_now = itr->price;
    price_now.amount = itr->price.amount - (itr->price.amount/100*checkprice)*itr->changeprice;
    // check the price, if the price os less than 0, we leave it at 0.
    if ( price_now.amount > config_->min_price.amount ) {
      price_now.amount = price_now.amount*amount;
    }
    else {
      price_now.amount = config_->min_price.amount*amount;
    }
    // check payment founds
    check ( quantity.amount >= price_now.amount , "You don't have enough founds");
    //transfer tokens
    action(
          permission_level{CONTRACTN, name("active")},
          itr->contract_adress,
          "transfer"_n,
          make_tuple(get_self(), claimer, asset{amount_, itr->maxsupply.symbol}, string("Buy tokens")))
        .send();
    // change number of purchases
    _tokendrop.modify(itr, claimer, [&](auto& row) {
      row.supply.amount += amount_;
    });
    users_config.modify(itr_user, claimer, [&](auto& row) {
      row.balance.amount -= price_now.amount;
    });
    // sending funds to the creator of the drop
    in_contract_transfer(itr->username, price_now);
  }
}