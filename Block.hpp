#ifndef BLOCK_HPP_
#define BLOCK_HPP_

#include <string>
#include <vector>
#include <algorithm>
#include <functional>

#include "Transaction.hpp"
#include "sha256.hpp"
#include "nlohmann/json.hpp"
#include "timestamp_as_string.hpp"

class Block
{
    private:
    std::uint64_t index;
    std::string blockHash;
    std::string previousBlockHash;
    std::uint64_t nounce = 0;
    std::uint32_t maxTransactionsCount = 3;
    std::uint32_t difficulty;
    std::vector<Transaction> transactions;
    std::string transactionsHashesSum = "";
    std::string minedAt = "not mined yet";

    public:
    Block(std::uint64_t index, std::string previousBlockHash, std::uint32_t difficulty, std::uint32_t maxTransactionsCount, Transaction transaction)
    {
        this->index = index;
        this->previousBlockHash = previousBlockHash;
        this->difficulty = difficulty;
        this->maxTransactionsCount = maxTransactionsCount;
        transactions.push_back(transaction);
    }

    Block(nlohmann::json block_json)
    {
        this->index = block_json["index"];
        this->blockHash = block_json["blockHash"];
        this->previousBlockHash = block_json["previousBlockHash"];
        this->nounce = block_json["nounce"];
        this->maxTransactionsCount = block_json["maxTransactionCount"];
        this->difficulty = block_json["difficulty"];
        this->minedAt = block_json["minedAt"];

        for(const auto & transaction : block_json["transactions"])
            transactions.push_back(Transaction(transaction));
    }

    constexpr friend std::ostream& operator<<(std::ostream& os, const Block& block)
    {
        return os << block.index;
    }

    bool push_transaction(Transaction transaction)
    {
        if(transactions.size() + 1 > maxTransactionsCount)
            return false;

        transactions.push_back(transaction);
        return true;
    }

    std::string calculateHash() 
    {
        transactionsHashesSum.clear();

        std::for_each(transactions.begin(), transactions.end(), [&](Transaction & transaction){
            transactionsHashesSum = sha256(transactionsHashesSum + transaction.getHash());
        });

        return sha256(transactionsHashesSum + previousBlockHash + std::to_string(index) + std::to_string(nounce));
    }

    void mine(std::uint32_t difficulty = 1)
    {
        std::cout << "Starting mining of block " << index << '\n';
        const auto miningStartTime = std::chrono::system_clock::now();

        do
        {
            ++nounce;
            blockHash = calculateHash();
        } while (!std::all_of(blockHash.begin(), blockHash.begin() + difficulty, [](char & a){
                return a == '0';
            }));

        minedAt = getTimestampAsString(std::chrono::system_clock::now());
        std::cout << "Block "<< index << " mined, nounce " << nounce << ", hash: " << blockHash << '\n'; 
        std::cout << "Took " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - miningStartTime).count() << " milliseconds to mine block\n";
    }

    bool validate() 
    {
        return this->blockHash == calculateHash();
    }

    std::string previousHash() const 
    {
        return previousBlockHash;
    }

    void dump(nlohmann::json & output_json) const
    {
        nlohmann::json block_json = {{"index", index}, {"blockHash", blockHash}, {"previousBlockHash", previousBlockHash}, {"nounce", nounce}, {"maxTransactionCount", maxTransactionsCount}, {"difficulty", difficulty}, {"minedAt", minedAt}, {"transactions", nlohmann::json::array({})}};
        std::for_each(transactions.begin(), transactions.end(), std::bind(std::mem_fn(&Transaction::dump), std::placeholders::_1, std::ref(block_json)));
        output_json["blocks"].push_back(block_json);
        /*std::cout << "\n\n### Start of Block " << index << ":\n";
        std::cout << "Block Nounce: " << nounce << "\n";
        std::cout << "Difficulty: " << difficulty << "\n";
        std::cout << "Block Hash: " << blockHash << "\n";
        std::cout << "Previous Block Hash: " << previousBlockHash << "\n";
        std::cout << "Mined At: " << minedAt << "\n\n";
        std::for_each(transactions.begin(), transactions.end(), std::bind(std::mem_fn(&Transaction::dump),std::placeholders::_1));
        std::cout << "### End of Block " << index << "\n\n";*/
        return;
    }

};

#endif
