#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>


class BankAccount
{
private:
	std::string name_;
	long long int balance_;
	std::mutex mtx_;

public:
	BankAccount(std::string name, int balance)
	{
		name_ = name;
		balance_ = balance;
	}
	BankAccount(BankAccount &&other)
	{
		name_ = other.name_;
		balance_ = other.balance_;
	}

	long long int deposit(int amount)
	{
		// Prevent depositing negative amount.
		if (amount <= 0)
			throw std::runtime_error("Deposit amount must be greater than 0.");
		std::lock_guard<std::mutex> lock(mtx_);

		balance_ += amount;
		return balance_;
	}

	long long int withdraw(int amount)
	{
		// Prevent withdrawing negative amount.
		if (amount <= 0)
			throw std::runtime_error("Withdraw amount must be greater than 0.");
		if (balance_ < amount)
			throw std::runtime_error("Insufficient balance.");
		std::lock_guard<std::mutex> lock(mtx_);

		balance_ -= amount;
		return balance_;
	}

	bool transferAmount(BankAccount &to, int amount)
	{
		bool withdrawSuccess = false;
		bool depositSuccess = false;

		// Withdraw from this account
		try
		{
			withdraw(amount);
			withdrawSuccess = true;
		}
		catch (std::runtime_error &e)
		{
			return false;
		}

		// Deposit to the other account
		try
		{
			to.deposit(amount);
			depositSuccess = true;
		}
		catch (std::runtime_error &e)
		{
			deposit(amount);
		}

		return withdrawSuccess && depositSuccess;
	}

	long long int getBalance()
	{
		std::lock_guard<std::mutex> lock(mtx_);
		return balance_;
	}

	std::string getName()
	{
		std::string copyOfName = name_; // Using the assignment operator to create a copy of name_
		return copyOfName;
	}
};

void doRandomOperations(std::vector<BankAccount> &bankAccounts, long long int randomOperations, bool logOperations = false)
{
	std::vector<std::string> operations = {"deposit", "withdraw", "transfer"};

	for (int i = 0; i < randomOperations; i++)
	{
		BankAccount &fromAccount = bankAccounts.at(rand() % bankAccounts.size());
		BankAccount &toAccount = bankAccounts.at(rand() % bankAccounts.size());
		long long int beforeOperationBalanceOnFromAccount = fromAccount.getBalance();
		long long int beforeOperationBalanceOnToAccount = toAccount.getBalance();

		int randomOperation = rand() % operations.size();
		int randomAmount = (rand() % 10); // Random amount between 1 and 200.

		try
		{
			if (operations.at(randomOperation) == std::string("deposit"))
			{
				fromAccount.deposit(randomAmount);
			}
			else if (operations.at(randomOperation) == std::string("withdraw"))
			{
				fromAccount.withdraw(randomAmount);
			}
			else
			{
				fromAccount.transferAmount(toAccount, randomAmount);
			}
		}
		catch (std::runtime_error &e)
		{
			if (logOperations)
			{
				std::cerr << e.what() << std::endl;
			}
		}

		if (logOperations)
		{
			if (operations[randomOperation] == std::string("transfer"))
			{
				std::cout << operations[randomOperation] << " \t " << randomAmount << " \t From " << fromAccount.getName() << ": " << beforeOperationBalanceOnFromAccount << "->" << fromAccount.getBalance() << " \t To " << toAccount.getName()
						  << ": " << beforeOperationBalanceOnToAccount << "->" << toAccount.getBalance() << std::endl;
			}
			else
			{
				std::cout << operations[randomOperation] << " \t " << randomAmount << " \t From " << fromAccount.getName()
						  << ": " << beforeOperationBalanceOnFromAccount << "->" << fromAccount.getBalance() << std::endl;
			}
			std::cout << std::endl;
		}
	}
}

int main()
{
	// Seed the random seed for number generator.
	std::srand(time(NULL));

	// Create two bank accounts with initial balance of 100 and 200.
	std::vector<BankAccount> bankAccounts;

	bankAccounts.push_back(BankAccount(std::string("A"), 0));
	bankAccounts.push_back(BankAccount(std::string("B"), 0));

	std::cout << "Starting balance of the two accounts:" << std::endl;
	std::cout << "\t" << bankAccounts.at(0).getName() << " -> " << bankAccounts.at(0).getBalance() << std::endl;
	std::cout << "\t" << bankAccounts.at(1).getName() << " -> " << bankAccounts.at(1).getBalance() << std::endl;

	long long int randomOperations = (rand() % 10000) + 1;

	// Start four threads that will make random deposit, withdraw and transfer transactions between the two accounts.
	std::thread t1(doRandomOperations, std::ref(bankAccounts), randomOperations, true);
	// std::thread t2(doRandomOperations, std::ref(bankAccounts), randomOperations, false);
	// std::thread t3(doRandomOperations, std::ref(bankAccounts), randomOperations, false);
	// std::thread t4(doRandomOperations, std::ref(bankAccounts), randomOperations, false);

	// Join threads
	t1.join();
	// t2.join();
	// t3.join();
	// t4.join();

	// Print the balance of the two accounts.
	std::cout << "Ending balance of the two accounts after " << randomOperations << " random transactions from 4 threads:" << std::endl;
	std::cout << "\t" << bankAccounts.at(0).getName() << " -> " << bankAccounts.at(0).getBalance() << std::endl;
	std::cout << "\t" << bankAccounts.at(1).getName() << " -> " << bankAccounts.at(1).getBalance() << std::endl;
	return 0;
}
