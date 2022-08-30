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
#define RAMFEE 0.995
#define CONTRACTN name("balancertest")
#define EOSIO name("eosio")
using namespace std;
using namespace eosio;

CONTRACT honefiassets : public contract {
  public:
    using contract::contract;
     honefiassets(name receiver, name code, datastream<const char*> ds):
        contract(receiver, code, ds),
        drop_table(receiver, receiver.value),
        config_table(receiver, receiver.value),
        users_config(receiver, receiver.value),
        _rambalance(receiver,receiver.value),
        _rammarket(EOSIO, EOSIO.value),
        _tokendrop(receiver, receiver.value){}
    // registration user
      ACTION cnftdrop(
        int changeprice,
        int changepricetime, 
        string format, 
        name username, 
        name collection, 
        name shemas, 
        uint32_t templates, 
        asset price, 
        int supply, 
        uint64_t dropstart, 
        uint64_t dropend, 
        string img,
        string drop_name, 
        string description);
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
      ACTION claimdrop ( name claimer, int drop_id, int claim_amount );
      ACTION buyram( name username, name collection,  asset quant );
      ACTION claimbalance( name username );      
      ACTION ctokendrop(
        name username, 
        name collection,
        string tokenticker,
        name contract_adress,
        int changeprice,
        int changepricetime, 
        int maxbuy_tx,
        string format,
        asset price, 
        int supply, 
        uint64_t dropstart, 
        uint64_t dropend, 
        string img,
        string drop_name, 
        string description);

  private:
    TABLE tokendrop {
      int dropnum;
      name username;
      name collection;
      string tokenticker;
      name contract_adress;
      int changeprice;
      int changepricetime; 
      int maxbuy_tx;
      string format;
      asset startprice;
      asset price;
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
    typedef multi_index<name("tokendrops"), tokendrop> tokendrops;
    tokendrops _tokendrop;
    //Connector
    struct connector_item
    {
        asset balance;
        double weight;
    };
    typedef connector_item connector;
    //RAM TABLE
    struct exchange_state
    {
        asset supply;
        connector base;
        connector quote;
        auto primary_key() const { return supply.amount; };
    };
    typedef multi_index<"rammarket"_n, exchange_state> rammarket;
    rammarket _rammarket;
    TABLE user {
        name username;
        float slipage;
        asset balance;
        auto primary_key() const { return username.value; }
      };
      typedef multi_index<name("userbalance"), user> users;
      users users_config;
      //RAM balance table
      TABLE rambalance {
        name collection;
        uint64_t bytes;

        auto primary_key() const { return collection.value; }
      };
      typedef multi_index<name("rambalance"), rambalance> rambalances;
      rambalances _rambalance;
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
    void in_contract_claim(name username, int drop_id, asset quantity);
    void in_contract_transfer(name username, asset quantity);
    void buyramproxy(name collection, asset quantity){
      // Call eosio contract to buy RAM
      action(
          permission_level{CONTRACTN, name("active")},
          "eosio"_n,
          "buyram"_n,
          make_tuple(CONTRACTN, CONTRACTN, quantity))
          .send();
              
      // Get RAM price to update balances
      double payRam = quantity.amount * RAMFEE;
      auto itrRamMarket = _rammarket.begin();
      double costRam = itrRamMarket->quote.balance.amount / itrRamMarket->base.balance.amount;
      uint64_t quotaRam = uint64_t(payRam / costRam);
        
      // Update user RAM amount
      auto itrBalance = _rambalance.find(collection.value);
      if (itrBalance == _rambalance.end())
      {
        _rambalance.emplace(_self, [&](auto &rec) {
            rec.collection = collection;
            rec.bytes = quotaRam;
        });
      }
      else
      {
        _rambalance.modify(itrBalance, _self, [&](auto &rec) {
            rec.bytes = itrBalance->bytes + quotaRam;
        });
      }
    }
  };
