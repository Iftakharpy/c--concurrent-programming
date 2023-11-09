#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <atomic>
#include <iomanip>

// Bank account with interest after every 100 transactions, using atomic operations


// Found how to do atomic multiply from https://www.modernescpp.com/index.php/atomics/
// Any calculation can be done this way
template <typename T>
T fetch_mult(std::atomic<T>& shared, T mult){
  T oldValue= shared.load();
  while (!shared.compare_exchange_strong(oldValue, oldValue * mult));
  return oldValue;
}


class BankAccount
{
private:
	std::string name_;
	std::atomic<double> balance_;
	std::atomic_ullong numberOfTransactions_ = {0};

	double updateNumberOfTransactions()
	{
		numberOfTransactions_++;
		auto TransactionCount = numberOfTransactions_.load();
		if(logActions) std::cout << "TransactionCount: " << TransactionCount << std::endl;
		if (TransactionCount%100==0 && numberOfTransactions_!=0){
			auto oldBalance = balance_.load();
			auto interest = oldBalance * (0.05/100); // 0.05% interest
			auto newBalance = oldBalance + interest;
			while (!balance_.compare_exchange_strong(oldBalance, newBalance));
			if (logActions){
				std::cout << "AddInterest\t" << interest << "\t"<< name_ << ": " << oldBalance<<"->"<<newBalance << "\t After "<< TransactionCount << " transactions" << std::endl;
			}
		}
		return balance_.load();
	}

public:
	bool logActions = false;

	BankAccount(std::string name, int balance, bool logActions = false)
	{
		name_ = name;
		balance_ = balance;
		logActions = logActions;
	}
	BankAccount(BankAccount &&other)
	{
		name_ = other.name_;
		balance_.store(other.balance_);
		numberOfTransactions_.store(other.numberOfTransactions_);
	}

	auto deposit(int amount)
	{
		// Prevent depositing negative amount.
		if (amount <= 0)
			throw std::runtime_error("Error: Deposit amount must be greater than 0.");

		auto oldBalance = balance_.load(); // Get the current balance
		auto newBalance = oldBalance + amount; // Calculate the new balance
		// To make sure the the operation is atomic we try find a state where the balance is the same as
		// the state when we have read it. When it matches the state we replace the old balance with the new balance.
		while (!balance_.compare_exchange_strong(oldBalance, newBalance));
		if (logActions){
			std::cout << "Deposit \t" << amount << "\t"<< name_ << ": " << oldBalance<<"->"<<newBalance << std::endl;
		}
		updateNumberOfTransactions();

		return balance_.load(); // Return the updated balance
	}

	auto withdraw(int amount)
	{
		// Prevent withdrawing negative amount.
		if (amount <= 0)
			throw std::runtime_error("Error: Withdraw amount must be greater than 0.");

		auto oldBalance = balance_.load(); // Get the current balance
		if (oldBalance < amount)
			throw std::runtime_error("Error: Insufficient balance.");

		auto newBalance = oldBalance - amount; // Calculate the new balance
		// To make sure the the operation is atomic we try find a state where the balance is the same as
		// the state when we have read it. When it matches the state we replace the old balance with the new balance.
		while (!balance_.compare_exchange_strong(oldBalance, newBalance));
		if (logActions){
			std::cout << "Withdraw \t" << amount << "\t"<< name_ << ": " << oldBalance<<"->"<<newBalance << std::endl;
		}
		updateNumberOfTransactions();

		return balance_.load(); // Return the updated balance
	}

	bool transferAmount(BankAccount &to, int amount)
	{
		if (to.getName() == name_)
			throw std::runtime_error("Error: Cannot transfer to the same account.");

		bool withdrawSuccess = false;
		bool depositSuccess = false;

		// Withdraw from this account
		auto fromOldBalance = balance_.load();
		auto toOldBalance = to.balance_.load();
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
		if (withdrawSuccess && depositSuccess)
			if(logActions) {
				std::cout << "Transfer \t" << amount << "\t" << "From " << name_ << ": " << fromOldBalance<<"->"<<balance_.load() <<"\t To "<<to.getName()<<": "<<toOldBalance<<"->"<<to.getBalance() << std::endl;
			}
			updateNumberOfTransactions();
			return true;
		return false;
	}

	double getBalance()
	{
		return balance_.load();
	}

	std::string getName()
	{
		std::string copyOfName = name_; // Using the assignment operator to create a copy of name_
		return copyOfName;
	}
};

void doRandomOperations(std::vector<BankAccount> &bankAccounts, long long int randomOperations, bool logActions = false)
{
	std::vector<std::string> operations = {"deposit", "withdraw", "transfer"};

	for (int i = 0; i < randomOperations; i++)
	{
		BankAccount &fromAccount = bankAccounts.at(rand() % bankAccounts.size());
		BankAccount &toAccount = bankAccounts.at(rand() % bankAccounts.size());
		fromAccount.logActions = logActions;
		toAccount.logActions = logActions;

		long long int beforeOperationBalanceOnFromAccount = fromAccount.getBalance();
		long long int beforeOperationBalanceOnToAccount = toAccount.getBalance();

		int randomOperation = rand() % operations.size();
		int randomAmount = (rand() % 20); // Random amount between 1 and 200.

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
			if (logActions)
			{
				std::cerr << e.what() << std::endl;
			}
		}
	}
}

int main()
{
	// Seed the random seed for number generator.
	std::srand(time(NULL));

	std::cout << std::setprecision(10);
	bool logActions = true;

	// Create two bank accounts with initial balance of 100 and 200.
	std::vector<BankAccount> bankAccounts;
	bankAccounts.push_back(BankAccount(std::string("A"), 0));
	bankAccounts.push_back(BankAccount(std::string("B"), 0));
	for (auto &account : bankAccounts)
	{
		account.logActions = logActions;
	}

	std::cout << "Starting balance of the two accounts:" << std::endl;
	std::cout << "\t" << bankAccounts.at(0).getName() << " -> " << bankAccounts.at(0).getBalance() << std::endl;
	std::cout << "\t" << bankAccounts.at(1).getName() << " -> " << bankAccounts.at(1).getBalance() << std::endl;

	long long int randomOperations = (rand() % 10000) + 1;
	// long long int randomOperations = 500000;

	// Start four threads that will make random deposit, withdraw and transfer transactions between the two accounts.
	std::thread t1(doRandomOperations, std::ref(bankAccounts), randomOperations, logActions);
	std::thread t2(doRandomOperations, std::ref(bankAccounts), randomOperations, logActions);
	std::thread t3(doRandomOperations, std::ref(bankAccounts), randomOperations, logActions);
	std::thread t4(doRandomOperations, std::ref(bankAccounts), randomOperations, logActions);

	// Join threads
	t1.join();
	t2.join();
	t3.join();
	t4.join();

	// Print the balance of the two accounts.
	std::cout << "Ending balance of the two accounts after " << randomOperations << " random transactions from 4 threads:" << std::endl;
	std::cout << "\t" << bankAccounts.at(0).getName() << " -> " << bankAccounts.at(0).getBalance() << std::endl;
	std::cout << "\t" << bankAccounts.at(1).getName() << " -> " << bankAccounts.at(1).getBalance() << std::endl;
	return 0;
}
