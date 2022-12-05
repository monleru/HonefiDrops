#include <honefiassets.hpp>

#include "nftdrop.cpp"
#include "tokendrop.cpp"

// registration new user
ACTION honefiassets::newuser(name username){
  require_auth(username);
  auto itr = users_config.find(username.value);
  check ( itr == users_config.end(), "You already registr");
  users_config.emplace(username, [&](auto& row) {
    row.username = username;
    row.slipage = 2.5;
  });
}
// slippage
ACTION honefiassets::setslip(name username, float slip){
  require_auth(username);
  check ( slip <= 50, "Maximum slip 50%");
  auto itr = users_config.find(username.value);
  check ( itr != users_config.end(), "You need registr");
  users_config.modify(itr, username, [&](auto& row) {
    row.slipage = slip;
  });
}

void honefiassets::deposit_token(name from,name to, asset quantity, string memo){
  // auth user
  require_auth(from);
  // check transfer wallet
  if (to != get_self() || from == get_self()) return;
  auto itr_user = users_config.find(from.value);
  if ( itr_user == users_config.end() ) {
    users_config.emplace(get_self(), [&](auto& row) {
      row.username = from;
      row.slipage = 2.5;
      row.balance = quantity;
    });
  }
  else{
    users_config.modify(itr_user, get_self(), [&](auto& row) {
      row.balance.amount += quantity.amount;
    });
  }
  require_recipient(from);
}
//User ram

// ...
ACTION honefiassets::dropremove(int drop_id){
  auto itr_drop = drop_table.find(drop_id);
  require_auth(itr_drop->username);
  itr_drop = drop_table.erase(itr_drop);
}
ACTION honefiassets::tdropremove(int drop_id){
  auto itr_drop = _tokendrop.find(drop_id);
  require_auth(itr_drop->username);
  itr_drop = _tokendrop.erase(itr_drop);
}
//set config
ACTION honefiassets::config(asset min_price){
  require_auth(get_self());
  config_table.emplace(get_self(), [&](auto& row) {
    row.username = get_self();
    row.dropnum = 0;
    row.min_price = min_price;
  });
}
//mint

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
ACTION honefiassets::buyram( name username, name collection,  asset quant ) {
  require_auth(username);
  auto itr_user = users_config.find(username.value);
  check ( quant.amount <= itr_user->balance.amount, "You don't have enough founds");
  buyramproxy(collection, quant);
  users_config.modify(itr_user, username, [&](auto& row) {
    row.balance.amount -= quant.amount;
  });
}
ACTION honefiassets::claimbalance( name username ){
  require_auth(username);
  auto itr_user = users_config.find(username.value);
  check ( itr_user != users_config.end(), "You don't have founds");
  check ( itr_user->balance.amount > 0, "You don't have founds");
  in_contract_transfer(username, itr_user->balance);
  users_config.modify(itr_user, username, [&](auto& row) {
    row.balance.amount = 0;
  });
}