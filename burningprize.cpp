#include <eosio/asset.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>
//#include <eosio/print.hpp>
#include <vector>
#include <math.h>

using namespace eosio;

class [[eosio::contract]] burningprize : public eosio::contract {
    
    using contract::contract;

    private:
        static constexpr symbol vex_symbol = symbol(symbol_code("VEX"), 4);
        //static constexpr symbol ram_symbol     = symbol(symbol_code("RAM"), 0);
    
        struct account
        {
            asset balance;
            uint64_t primary_key() const {return balance.symbol.code().raw();}
        };

        typedef eosio::multi_index< eosio::name("accounts"), account > accounts;

        struct member {
            name account_name;
            asset quantity;

            uint64_t primary_key()const { return account_name.value; }

            EOSLIB_SERIALIZE( member, (account_name)(quantity) )
        };

        typedef eosio::multi_index< "members"_n, member > members;


        uint32_t now() {
            return current_time_point().sec_since_epoch();
        }


        uint32_t random(uint64_t some_seed, uint64_t upper_limit) {
            return ( now() + some_seed ) % upper_limit;
        }


    public:

        // @abi table winner
        struct [[eosio::table]] winner {
            uint32_t id;
            name account_name;
            uint32_t timestamp;
            asset quantity;
            bool is_final;

            uint32_t primary_key()const { return id; }
            //uint64_t primary_key()const { return account_name.value; }

            //EOSLIB_SERIALIZE( winner, (account_name)(is_final) )
            EOSLIB_SERIALIZE( winner, (id)(account_name)(timestamp)(quantity)(is_final) )
        };

        typedef eosio::multi_index< "thewinner"_n, winner > thewinner;


        [[eosio::action]]
        void getversion() {
            print("BurningPrize SC v1.8 - databisnisid - 20200806\t");
        }

        [[eosio::action]]
        void cleartable() {
            require_auth(_self);

            thewinner _thewinner(get_self(), get_self().value);

            auto itr = _thewinner.begin();
            while (itr != _thewinner.end()) {
                itr = _thewinner.erase(itr);
            }
            print("All records are deleted!");
        }


        [[eosio::action]]
        void randomwinner( uint32_t randomnumber ) {
            require_auth(_self);


            // Get account vex.saving balance
            accounts _accounts("vex.token"_n, "vex.saving"_n.value);
            const auto sym_name = symbol_code("VEX");
            const auto& vexsaving = _accounts.get( sym_name.raw() );
            print("vex.saving balance is ", vexsaving.balance, "\t");


            // if vex.saving is not burn, keep draw the winner
            if ( vexsaving.balance.amount / 10000 > 1000000 ) { 
                    members _members("registeronme"_n, "registeronme"_n.value);

                    uint64_t total_quantity = 0;
                    std::vector <name> allmembers;

                    for (auto itr = _members.begin(); itr != _members.end(); itr++)
                    {
                        // create vector of all members with factor the weight
                        for( auto ci = 1; ci <= floor(itr->quantity.amount / 10000); ci++ ) {
                            allmembers.push_back( itr->account_name );
                        }

                        total_quantity += itr->quantity.amount;
                    }

                    print("Total quantity is ", total_quantity, "\t");

                    /*
                    for( auto cj = 0; cj < allmembers.size(); cj++ ) {
                        print(" ", allmembers[cj], " ");
                    }
                    */
                   
                   
                    // get random index from vector allmembers
                    int result = random(randomnumber, allmembers.size());

                    auto itr = _members.find( allmembers[result].value);

                    auto cow = ( itr->quantity.amount / total_quantity ) * 100;

                    print("Random Winner is ", allmembers[result], " with Random Number ", randomnumber, "\t");

                    
                    // Update table winner
                    thewinner _thewinner(get_self(), get_self().value);
                    auto itr1 = _thewinner.find( 1 );

                    if( itr1 != _thewinner.end() ) {
                        _thewinner.modify( itr1, get_self(), [&](auto &row) {
                            row.account_name = allmembers[result];
                            row.timestamp = now();
                            row.quantity.amount = itr->quantity.amount;
                            row.quantity.symbol = itr->quantity.symbol;
                            row.is_final = false;
                        });
                    } else {
                        _thewinner.emplace( get_self(), [&](auto &row){
                            row.id = 1;
                            row.account_name = allmembers[result];
                            row.timestamp = now();
                            row.quantity.amount = itr->quantity.amount;
                            row.quantity.symbol = itr->quantity.symbol;
                            row.is_final = false;
                        });
                    }

            } else {
                    // Update table winner
                    thewinner _thewinner(get_self(), get_self().value);
                    auto itr1 = _thewinner.find( 1 );

                    if( itr1 != _thewinner.end() ) {
                        _thewinner.modify( itr1, get_self(), [&](auto &row) {
                            row.is_final = true;
                        });
                    } else {
                        _thewinner.emplace( get_self(), [&](auto &row){
                            row.is_final = true;
                        });
                    }

            }

        }    

};