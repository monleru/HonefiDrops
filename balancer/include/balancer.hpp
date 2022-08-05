#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/system.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>
#include <atomicassets-interface.hpp>
#include <vector>
#include <list>
#include <stdio.h>
#include <cmath>
#include <string>
#include <tuple>
using namespace std;
using namespace eosio;

CONTRACT balancer : public contract {
  public:
    using contract::contract;
     balancer(name receiver, name code, datastream<const char*> ds):
        contract(receiver, code, ds),
        drop_table(receiver, receiver.value),
        config_table(receiver, receiver.value),
        users_config(receiver, receiver.value){}
    // registration user
      ACTION createdrop(int dropnum, int changeprice,int changepricetime, string format, name username, name collection, name shemas, uint32_t templates, asset price, int supply, uint64_t dropstart, uint64_t dropend, string img,string drop_name, string description);
      [[eosio::on_notify("eosio.token::transfer")]]
      void token_transfer(name from,name to, asset asset_ids, string memo);
      ACTION newuser(name username);
      ACTION setslip(name username, float slip);
      ACTION dropremove(int drop_id);
      ACTION config();
      ACTION setdroptime(int drop_id, uint64_t dropstart, uint64_t dropend);
      ACTION setprice(int drop_id, asset price);
      ACTION setinfo(int drop_id, string img, string drop_name, string description);
      ACTION setsupply( int drop_id, int supply);
      ACTION setchangepr( int drop_id, int changeprice,int changepricetime);
      


  private:
    TABLE user {
        name username;
        float slip;
        auto primary_key() const { return username.value; }
      };
      typedef multi_index<name("users"), user> users;
      users users_config;
    TABLE drop {
          int dropnum;
          string format;
          name username;
          name collection;
          name shemas;
          uint32_t templates;
          asset startprice;
          asset price;
          int changepricetime;
          int changeprice;
          int supply;
          int maxsupply;
          uint64_t dropstart;
          uint64_t dropend;
          uint64_t lastbuy;
          string img;
          string drop_name;
          string description;
          auto primary_key() const { return dropnum; }
        };
        typedef multi_index<name("dropsssl"), drop> drop_tabl;
        drop_tabl drop_table;
    TABLE dropconfig {
          name username;
          int dropnum;
          auto primary_key() const { return username.value; }
        };
        typedef multi_index<name("config"), dropconfig> configs;
        configs config_table;
    void in_contract_claim(name username, string memo, asset quantity);
    void in_contract_transfer(name username, asset quantity);

};
