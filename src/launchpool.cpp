#include <honefiassets.hpp>

ACTION honefiassets::createlpool(
    name contract_adress,
    asset supply,
    asset price,
    asset deposit,
    int token_decimals,
    uint64_t start,
    uint64_t end_,
    string token_ticker) {
    require_auth(get_self());
    auto itr_dropnum = config_table.find(get_self().value);

    _launchpool.emplace(get_self(), [&](auto& row) {
        row.dropnum = itr_dropnum->dropnum + 1;
        row.contract_adress = contract_adress;
        row.supply = supply;
        row.price = price;
        row.deposit = deposit;
        row.start = start;
        row.token_decimals = token_decimals;
        row.end_ = end_;
        row.token_ticker = token_ticker;
        row.approve = false;
    });
    

    config_table.modify(itr_dropnum, get_self(), [&](auto& row) {
        row.dropnum += 1;
    });
}
ACTION honefiassets::stakelaunch(name username,int dropnum, asset quantity){
    require_auth(username);


    auto itr_launch = _launchpool.find(dropnum);
    check ( itr_launch != _launchpool.end(),"Invalid dropnum");
    //get Time
    auto now = current_time_point().sec_since_epoch();
    check ( now >= itr_launch->start, "Launchpool didnt start");
    check ( now <= itr_launch->end_, "Launchpool end");

    //check Balance
    auto itr_user = users_config.find(username.value);
    check (itr_user != users_config.end(),"You don't have enoght founds");
    check ( itr_user-> balance.amount >= quantity.amount, "You don't have enoght founds");
    users_config.modify(itr_user, username, [&](auto& row){
        row.balance.amount -= quantity.amount;
    });
    _launchpool.modify(itr_launch, username, [&](auto& row){
        row.deposit.amount += quantity.amount;
    });

    vector <_stakeitem> stakeitem_ = {};

    stakeitem_.push_back({
        .balance = quantity,
        .dropnum = dropnum
    });



    auto itr_launch_stake = _launchstake.find(username.value);
    if (itr_launch_stake == _launchstake.end() ) {
        _launchstake.emplace(username, [&](auto& row){
            row.username = username;
            row.stakeitem = stakeitem_;
        });
    } else{ 
        vector<_stakeitem> stake__ = itr_launch_stake->stakeitem;
        asset stake_amount;
        bool staked = false;
        int i_ = 0;
        for ( int i = 0; i < itr_launch_stake->stakeitem.size(); i++ ) {
            if ( itr_launch_stake->stakeitem[i].dropnum == dropnum ) {
                stake_amount = itr_launch_stake->stakeitem[i].balance;
                staked = true;
                i_ = i;
                break;
            }
        }
        if ( staked == true ) {
            stake__[i_].balance.amount = stakeitem_[0].balance.amount + stake_amount.amount;
            _launchstake.modify(itr_launch_stake, username, [&](auto& row){
                row.stakeitem = stake__;
            });

        } else {
            stake__.push_back(stakeitem_[0]);
            _launchstake.modify(itr_launch_stake, username, [&](auto& row){
                row.stakeitem = stake__;
            });
        }
    }   
}
ACTION honefiassets::claimlaunch( name username, int dropnum ) {
    require_auth(username);
    auto itr_launch = _launchpool.find(dropnum);
    check ( itr_launch != _launchpool.end(),"Invalid dropnum");
    auto now = current_time_point().sec_since_epoch();
    check ( now >= itr_launch->end_, "drop didn't end");
    //get username percent pool
    auto itr_launch_stake = _launchstake.find(username.value);
    check(itr_launch_stake != _launchstake.end(), "You don't have enoguht funds");
    bool staked = false;
    asset deposit;
    for ( int i = 0; i < itr_launch_stake->stakeitem.size(); i++ ) {
        if (itr_launch_stake->stakeitem[i].dropnum == dropnum) {
            deposit = itr_launch_stake->stakeitem[i].balance;
            staked = true;
            auto stakeitem__ = itr_launch_stake->stakeitem;
            stakeitem__.erase(stakeitem__.begin() + i);
            _launchstake.modify(itr_launch_stake, username, [&](auto& row){
                row.stakeitem = stakeitem__;
            });
            break;
        }
    }
    check (staked == true, "You don't have enoght founds");
    float percent = deposit.amount/itr_launch->deposit.amount*100;
    asset buy_token = itr_launch->supply;
    buy_token.amount = buy_token.amount/100*percent;
    //send token
    action(
          permission_level{CONTRACTN, name("active")},
          itr_launch->contract_adress,
          "transfer"_n,
          make_tuple(get_self(), username, buy_token, string("Claim launchpool tokens")))
        .send();

    //return wax
    int maxsupply = itr_launch->supply.amount/pow(10, itr_launch->token_decimals);
    asset maxdeposit = itr_launch->price;
    maxdeposit.amount = maxdeposit.amount*maxsupply;
    asset waxreturn = maxdeposit;
    waxreturn.amount = maxdeposit.amount - (maxdeposit.amount/100*percent);
    //send wax
    action(
          permission_level{CONTRACTN, name("active")},
          "eosio.token"_n,
          "transfer"_n,
          make_tuple(get_self(), username, waxreturn, string("Claim launchpool tokens")))
        .send();
}




ACTION honefiassets::claimwax(name username, int dropnum){
    require_auth(username);
        auto itr_launch = _launchpool.find(dropnum);
    check ( itr_launch != _launchpool.end(),"Invalid dropnum");
    //get Time
    auto now = current_time_point().sec_since_epoch();
    check ( now > itr_launch->end_, "Drop is live");
    check ( username == itr_launch->contract_adress, "Error auth");
    uint64_t supply__ = itr_launch->supply.amount/pow(10, itr_launch->token_decimals);
    uint64_t price__ = itr_launch->price.amount/pow(10,8);
    asset waxclaim = asset{0, itr_launch->price.symbol};
    if ( itr_launch->deposit.amount > (supply__*price__)*pow(10,8) ) {
        waxclaim.amount = (supply__*price__)*pow(10,8);
    } else {
        waxclaim.amount = itr_launch->deposit.amount;
    }
    //send WAX
    action(
          permission_level{CONTRACTN, name("active")},
          "eosio.token"_n,
          "transfer"_n,
          make_tuple(get_self(), username, waxclaim, string("Claim launchpool tokens")))
        .send();
}

