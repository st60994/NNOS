// Operating Systems: sample code  (c) Microsoft, Tomáš Hudec
// Threads, C#
// Critical Sections
//
// Concurrent Bank Transactions (Withdrawal) Problem and the Monitor
//
// Last modification: 2015-11-30
//
// URL of the original: https://msdn.microsoft.com/en-us/library/c5kehkcz.aspx

// use Mono to compile:
//	mcs Transactions.cs

using System;
using System.Threading;

// account balance
class Account
{
//	private Object thisLock = new Object();	// for monitor
	int balance;				// account balance

	Random r = new Random();

	public Account(int initial)		// set initial balance
	{
		balance = initial;
	}

	// withdraw given amount of money from the account
	int Withdraw(int amount)
	{
		// this condition is never true unless the lock statement is commented out
		if (balance < 0) {
			throw new Exception("Negative Balance");
		}

		// comment out the next line to see the effect of leaving out the lock keyword
//		lock (thisLock)			// uses Monitor
		{
			if (balance >= amount) {
				Console.WriteLine("Balance before Withdrawal:  " + balance);
				Console.WriteLine("Amount to Withdraw:        -" + amount);
				balance = balance - amount;
				Console.WriteLine("Balance after Withdrawal:   " + balance);
				return amount;
			}
			else
				return 0;	// transaction rejected
		}
	}

	// do 100 withdrawal transactions (random amounts)
	public void DoTransactions()
	{
		for (int i = 0; i < 100; ++i)
			Withdraw(r.Next(1, 100));
	}
}

class Test
{
	static void Main()
	{
		Thread[] threads = new Thread[10];	// 10 concurrent threads
		Account acc = new Account(1000);	// set initial balance

		// create threads to perform transactions
		for (int i = 0; i < 10; ++i) {
			Thread t = new Thread(new ThreadStart(acc.DoTransactions));
			threads[i] = t;
		}
		// run all the threads
		for (int i = 0; i < 10; ++i)
			threads[i].Start();
	}
}
