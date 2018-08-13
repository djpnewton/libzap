#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <iostream>

#include "../src/zap.h"
#include "args.hxx"

void print_hex(unsigned char *buf, size_t sz)
{
    for (int i = 0; i < sz; i++)
        printf("%02x", buf[i]);
}

void print_tx(struct tx_t tx, int i)
{
    printf("  tx-%d\n    id: %s\n    sender: %s\n    recipient: %s\n",
        i, tx.id, tx.sender, tx.recipient);
    printf("    asset id: %s\n    fee asset: %s\n    attachment: %s\n",
        tx.asset_id, tx.fee_asset, tx.attachment);
    printf("    amount: %lld\n    fee: %lld\n    timestamp: %lld\n",
        tx.amount, tx.fee, tx.timestamp);
}

std::vector<unsigned char> hex_str_to_vec(std::string &in)
{
    size_t len = in.length();
    std::vector<unsigned char> out;
    for(size_t i = 0; i < len; i += 2) {
        std::istringstream strm(in.substr(i, 2));
        int x;
        strm >> std::hex >> x;
        out.push_back(x);
    }
    return out;
}

int main(int argc, char *argv[])
{
    args::ArgumentParser p("libzap test app");
    args::Group commands(p, "commands");

    args::Command version(commands, "version", "show the libzap version");
    args::Command mnemonic(commands, "mnemonic", "create a bip39 mnemonic");
    args::Command mnemonic_check(commands, "mnemonic_check", "check a bip39 mnemonic");
    args::ValueFlag<std::string> mnemonic_opt(mnemonic_check, "mnemonic", "mnemonic to input", {'m'});
    args::Command seed_to_address(commands, "seed_to_address", "convert a seed to an address");
    args::ValueFlag<std::string> seed_opt(seed_to_address, "seed", "seed to input", {'s'});
    args::Command balance(commands, "balance", "balance of an address");
    args::ValueFlag<std::string> address_opt(balance, "address", "address to input", {'a'});
    args::Command transactions(commands, "transactions", "transactions of an address");
    args::ValueFlag<std::string> address_opt2(transactions, "address", "address to input", {'a'});
    args::Command create(commands, "create", "create spend transaction for an address");
    args::ValueFlag<std::string> seed_opt2(create, "seed", "address to input", {'s'});
    args::ValueFlag<long> amount_opt(create, "amount", "amount to send", {'a'});
    args::ValueFlag<std::string> recipient_opt(create, "recipient", "recipient to send to", {'r'});
    args::ValueFlag<std::string> attachment_opt(create, "attachment", "attachment for tx", {'A'});
    args::Command broadcast(commands, "broadcast", "broadcast spend transaction");
    args::ValueFlag<std::string> data_opt(broadcast, "data", "transaction data (hex string - no leading '0x')", {'d'});
    args::ValueFlag<std::string> sig_opt(broadcast, "signature", "signature data (hex string - no leading '0x')", {'s'});

    args::Group arguments(p, "arguments", args::Group::Validators::DontCare, args::Options::Global);
    args::ValueFlag<char> network(arguments, "network", "Mainnet or testnet (default 'T')", {'n'});
    args::HelpFlag h(arguments, "help", "help", {'h', "help"});

    try
    {
        // set the network
        lzap_network_set('T');
        if (network)
            lzap_network_set(args::get(network));

        p.ParseCLI(argc, argv);
        if (version)
        {
            // libzap version
            printf("libzap version %d\n", lzap_version());
        }
        else if (mnemonic)
        {
            // mnemonic
            char mnemonic[1024];
            bool res = lzap_mnemonic_create(mnemonic, 1024);
            printf("mnemonic create: %d\nmnemonic: %s\n", res, mnemonic);
        }
        else if (mnemonic_check)
        {
            // mnemonic check
            auto mnemonic = args::get(mnemonic_opt).c_str();
            printf("mnemonic check: %d\n", lzap_mnemonic_check(mnemonic));
            printf("mnemonic: %s\n", mnemonic);
        }
        else if (seed_to_address)
        {
            // seed to address
            auto seed = args::get(seed_opt).c_str();
            char address[1024];
            lzap_seed_address(seed, address);
            printf("seed: %s\naddress: %s\n", seed, address);
        }
        else if (balance)
        {
            // address balance
            auto address = args::get(address_opt).c_str();
            struct int_result_t result = lzap_address_balance(address);
            printf("address balance success: %d\naddress balance value: %lld\n", result.success, result.value);
        }
        else if (transactions)
        {
            // address transactions 
            auto address = args::get(address_opt2).c_str();
            struct tx_t txs[100];
            struct int_result_t result = lzap_address_transactions(address, txs, 100);
            printf("address transactions success: %d\n", result.success);
            if (result.success)
                for (int i = 0; i < result.value; i++)
                    print_tx(txs[i], i);
        }
        else if (create)
        {
            // spend tx
            auto seed = args::get(seed_opt2).c_str();
            auto recipient = args::get(recipient_opt).c_str();
            auto amount = args::get(amount_opt);
            auto attachment = args::get(attachment_opt).c_str();

            struct int_result_t fee = lzap_transaction_fee();
            assert(fee.success);
            printf("transaction fee: %llu\n", fee.value);
            struct spend_tx_t tx = lzap_transaction_create(seed, recipient, amount, fee.value, attachment);
            printf("transaction create:\n\tsuccess:   %d\n\tdata:      ", tx.success);
            print_hex((unsigned char*)tx.data, tx.data_size);
            printf("\n\tsignature: ");
            print_hex((unsigned char*)tx.signature, sizeof(tx.signature));
            printf("\n");
        }
        else if (broadcast)
        {
            // broadcast tx
            auto data_hex = args::get(data_opt);
            auto sig_hex = args::get(sig_opt);
            std::vector<unsigned char> data = hex_str_to_vec(data_hex);
            std::vector<unsigned char> sig_data = hex_str_to_vec(sig_hex);
            struct spend_tx_t tx = {};
            memcpy(&tx.data, &data[0], data.size());
            memcpy(&tx.signature, &sig_data[0], sig_data.size());
            struct tx_t broadcast_tx;
            bool result = lzap_transaction_broadcast(tx, &broadcast_tx);
            printf("transaction broadcast: %d\n", result);
            print_tx(broadcast_tx, 0);
        }

        std::cout << std::endl;
    }
    catch (args::Help)
    {
        std::cout << p;
    }
    catch (args::Error& e)
    {
        std::cerr << e.what() << std::endl << p;
        return 1;
    }
    return 0;
}
