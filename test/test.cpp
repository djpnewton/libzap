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

void print_lzap_error()
{
    int code;
    const char *msg;
    lzap_error(&code, &msg);
    if (code != LZAP_ERR_NONE)
    {
        printf("error code: %d, msg: %s\n", code, msg);
        exit(code);
    }
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
    args::Command mnemonic_wordlist(commands, "mnemonic_wordlist", "print all bip39 words");
    args::Command seed_address(commands, "seed_address", "convert a seed to an address");
    args::ValueFlag<std::string> seed_opt_addr(seed_address, "seed", "seed to input", {'s'});
    args::Command check(commands, "check", "check the validity of an address");
    args::ValueFlag<std::string> address_opt_check(check, "address", "address to input", {'a'});
    args::Command balance(commands, "balance", "balance of an address");
    args::ValueFlag<std::string> address_opt_balance(balance, "address", "address to input", {'a'});
    args::ValueFlag<std::string> seed_opt_balance(balance, "seed", "seed to input", {'s'});
    args::Command transactions(commands, "transactions", "transactions of an address");
    args::ValueFlag<std::string> address_opt_txs(transactions, "address", "address to input", {'a'});
    args::ValueFlag<std::string> seed_opt_txs(transactions, "seed", "seed to input", {'s'});
    args::Command create(commands, "create", "create spend transaction for an address");
    args::ValueFlag<std::string> seed_opt_create(create, "seed", "address to input", {'s'});
    args::ValueFlag<long> amount_top_create(create, "amount", "amount to send", {'a'});
    args::ValueFlag<std::string> recipient_opt_create(create, "recipient", "recipient to send to", {'r'});
    args::ValueFlag<std::string> attachment_opt_create(create, "attachment", "attachment for tx", {'A'});
    args::Command broadcast(commands, "broadcast", "broadcast spend transaction");
    args::ValueFlag<std::string> data_opt(broadcast, "data", "transaction data (hex string - no leading '0x')", {'d'});
    args::ValueFlag<std::string> sig_opt(broadcast, "signature", "signature data (hex string - no leading '0x')", {'s'});
    args::Command spend(commands, "spend", "convenience function to create and broadcast a transaction");
    args::ValueFlag<std::string> seed_opt_spend(spend, "seed", "address to input", {'s'});
    args::ValueFlag<long> amount_opt_spend(spend, "amount", "amount to send", {'a'});
    args::ValueFlag<std::string> recipient_opt_spend(spend, "recipient", "recipient to send to", {'r'});
    args::ValueFlag<std::string> attachment_opt_spend(spend, "attachment", "attachment for tx", {'A'});
    args::Command uriparse(commands, "uriparse", "parse a waves uri ('waves://<address>?asset=<assetid>&amount=<amount>&attachment=<attachment>')");
    args::ValueFlag<std::string> uri_opt(uriparse, "uri", "uri to input", {'u'});

    args::Group arguments(p, "arguments", args::Group::Validators::DontCare, args::Options::Global);
    args::ValueFlag<char> network(arguments, "network", "Mainnet ('W') or testnet ('T' - default)", {'n'});
    args::ValueFlag<std::string> node(arguments, "node", "Use a custom waves node", {'N'});
    args::HelpFlag h(arguments, "help", "help", {'h', "help"});

    try
    {
        p.ParseCLI(argc, argv);

        // set the network
        lzap_network_set('T');
        if (network)
        {
            printf("setting network\n");
            lzap_network_set(args::get(network));
            print_lzap_error();
        }

        // set the node
        if (node)
        {
            printf("setting node\n");
            lzap_node_set(args::get(node).c_str());
            print_lzap_error();
        }

        printf("Using network: %c, node: %s\n", lzap_network_get(), lzap_node_get());

        if (version)
        {
            // libzap version
            printf("libzap version %d\n", lzap_version());
            print_lzap_error();
        }
        else if (mnemonic)
        {
            // mnemonic
            char mnemonic[1024];
            bool result = lzap_mnemonic_create(mnemonic, 1024);
            print_lzap_error();
            printf("mnemonic create: %d\nmnemonic: %s\n", result, mnemonic);
        }
        else if (mnemonic_check)
        {
            // mnemonic check
            auto mnemonic = args::get(mnemonic_opt).c_str();
            bool result = lzap_mnemonic_check(mnemonic);
            print_lzap_error();
            printf("mnemonic check: %d\n", result);
            printf("mnemonic: %s\n", mnemonic);
        }
        else if (mnemonic_wordlist)
        {
            const char* const* words = lzap_mnemonic_wordlist();
            print_lzap_error();
            int i = 1;
            while (*words)
            {
                printf("%s ", *words);
                if (i % 12 == 0)
                    printf("\n");
                words++;
                i++;
            }
            printf("\n");
        }
        else if (seed_address)
        {
            // seed to address
            auto seed = args::get(seed_opt_addr).c_str();
            char address[1024];
            lzap_seed_address(seed, address);
            print_lzap_error();
            printf("seed: %s\naddress: %s\n", seed, address);
        }
        else if (check)
        {
            // address check
            auto address = args::get(address_opt_check).c_str();
            struct int_result_t result = lzap_address_check(address);
            print_lzap_error();
            printf("address check success: %d\naddress check value: %lld\n", result.success, result.value);
        }
        else if (balance)
        {
            // address balance
            char address_[1024] = {};
            const char *address = address_;
            if (address_opt_balance)
                address = args::get(address_opt_balance).c_str();
            else if (seed_opt_balance)
            {
                auto seed = args::get(seed_opt_balance).c_str();
                lzap_seed_address(seed, address_);
            }
            struct int_result_t result = lzap_address_balance(address);
            print_lzap_error();
            printf("address: %s\naddress balance success: %d\naddress balance value: %lld\n", address, result.success, result.value);
        }
        else if (transactions)
        {
            // address transactions 
            char address_[1024] = {};
            const char *address = address_;
            if (address_opt_txs)
                address = args::get(address_opt_txs).c_str();
            else if (seed_opt_txs)
            {
                auto seed = args::get(seed_opt_txs).c_str();
                lzap_seed_address(seed, address_);
            }
            struct tx_t txs[100];
            struct int_result_t result = lzap_address_transactions(address, txs, 100);
            print_lzap_error();
            printf("address: %s\naddress transactions success: %d\n", address, result.success);
            if (result.success)
            {
                for (int i = 0; i < result.value; i++)
                    print_tx(txs[i], i);
            }
        }
        else if (create)
        {
            // create tx
            auto seed = args::get(seed_opt_create).c_str();
            auto recipient = args::get(recipient_opt_create).c_str();
            auto amount = args::get(amount_top_create);
            auto attachment = args::get(attachment_opt_create).c_str();

            struct int_result_t fee = lzap_transaction_fee();
            print_lzap_error();
            printf("transaction fee: %llu\n", fee.value);
            struct spend_tx_t tx = lzap_transaction_create(seed, recipient, amount, fee.value, attachment);
            print_lzap_error();
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
            print_lzap_error();
            printf("transaction broadcast: %d\n", result);
            if (result)
                print_tx(broadcast_tx, 0);
        }
        else if (spend)
        {
            // create tx
            auto seed = args::get(seed_opt_spend).c_str();
            auto recipient = args::get(recipient_opt_spend).c_str();
            auto amount = args::get(amount_opt_spend);
            auto attachment = args::get(attachment_opt_spend).c_str();

            struct int_result_t fee = lzap_transaction_fee();
            print_lzap_error();
            printf("transaction fee: %llu\n", fee.value);
            struct spend_tx_t tx = lzap_transaction_create(seed, recipient, amount, fee.value, attachment);
            print_lzap_error();
            printf("transaction create:\n\tsuccess:   %d\n\tdata:      ", tx.success);
            print_hex((unsigned char*)tx.data, tx.data_size);
            printf("\n\tsignature: ");
            print_hex((unsigned char*)tx.signature, sizeof(tx.signature));
            printf("\n");
            struct tx_t broadcast_tx;
            bool result = lzap_transaction_broadcast(tx, &broadcast_tx);
            print_lzap_error();
            printf("transaction broadcast: %d\n", result);
            if (result)
                print_tx(broadcast_tx, 0);
        }
        else if (uriparse)
        {
            // parse uri
            auto uri = args::get(uri_opt).c_str();

            struct waves_payment_request_t req;
            bool result = lzap_uri_parse(uri, &req);
            print_lzap_error();
            printf("uri parse: %d\n  address: %s\n  asset_id: %s\n  attachment: %s\n  amount: %llu\n", result, req.address, req.asset_id, req.attachment, req.amount);
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
