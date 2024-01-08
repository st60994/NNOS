// Operating Systems: sample code  (c) Tomáš Hudec
// Threads, Java
// Critical Sections, Synchronization
//
// Producer/Consumer Problem TO BE SOLVED
//
// Modified: 2020-12-29, 2022-11-29

/* Zadání

Vyřešte problém konzumentů a producentů
bez aktivního čekání
užitím monitoru s podmínkovými proměnnými v jazyce Java.

Problémy:
-- synchronizace při prázdném a plném skladu,
-- kritická sekce při vstupu do skladu.

Použijte vhodné komentáře.

Výsledný program pojmenujte ProducerConsumerMonitor.java a vytvořte Makefile.

*/

/* Assignment

Solve the producer-consumer problem
with non-busy wait
using Java monitor with condition variables.

Problems:
-- synchronization if the shared storage is empty and full
-- race condition while entering the storage

Compilation:
	javac ProducerConsumer.java
Run:
	java ProducerConsumer

*/

import java.util.Random;


public class ProducerConsumer
{
	public static void main(String [] args)
	{
		int totalProducers  = 2;	// number of producers
		int totalConsumers  = 1; 	// number of consumers
		int storageCapacity = 5;	// storage capacity
		char [] [] toBeProducedData = {
			"abcdefghijklmnopqrstuvwxyz".toCharArray(),
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ".toCharArray(),
			"0123456789".toCharArray(),
			"!@#$%~&*()=+-[]{};:,<.>/?".toCharArray(),
		};
		final int maxConsumers = 100;
		final int maxCapacity = 10;
		char [] quitItems;
		char quitItem = '^';	// upon reading this item the consumer ends
		char emptyItem = '_';	// free place in a storage
		int i;
	
		// parse options
		// the program expects at most three numbers:
		//	1st: number of producers
		//	2nd: number of consumers
		//	3rd: storage capacity
		switch (args.length) {
			case 3:
				storageCapacity = Integer.parseInt(args[2]);
				if (storageCapacity > maxCapacity) {
					storageCapacity = maxCapacity;
					System.err.println("The storage capacity reduced to " + storageCapacity + ".");
				}
				if (storageCapacity < 1) {
					storageCapacity = 1;
					System.err.println("The storage capacity set to " + storageCapacity + ".");
				}
			case 2:
				totalConsumers = Integer.parseInt(args[1]);
				if (totalConsumers > maxConsumers) {
					totalConsumers = maxConsumers;
					System.err.println("The number of consumers reduced to " + totalConsumers + ".");
				}
				if (totalConsumers < 1) {
					totalConsumers = 1;
					System.err.println("The number of consumers set to " + totalConsumers + ".");
				}
			case 1:
				totalProducers = Integer.parseInt(args[0]);
				if (totalProducers > toBeProducedData.length) {
					totalProducers = toBeProducedData.length;
					System.err.println("The number of producers reduced to " + totalProducers + ".");
				}
				if (totalProducers < 1) {
					totalProducers = 1;
					System.err.println("The number of producers set to " + totalProducers + ".");
				}
			case 0:
				break;
			default:
				System.err.println("Wrong number of arguments.");
				System.exit(1);
		}

		// initialize the shared storage
		SharedStorage storage = new SharedStorage(storageCapacity, emptyItem);
		
		// initialize number of producers
		Producer [] p = new Producer[totalProducers];
		// initialize number of consumers
		Consumer [] c = new Consumer[totalConsumers];
		// for quitting
		quitItems = new char[c.length];

		// start consumers
		//System.err.println("Starting " + c.length + " consumer(s).");
		for (i = 0; i < c.length; ++i) {
			c[i] = new Consumer(storage, quitItem);
			c[i].start();
			quitItems[i] = quitItem; // fill the array for final producer
		}

		// start producers
		//System.err.println("Starting " + p.length + " producer(s).");
		for (i = 0; i < p.length; ++i) {
			p[i] = new Producer(storage, toBeProducedData[i]);
			p[i].start();
		}

		// wait for all producers
		for (i = 0; i < p.length; ++i) {
			try {
				p[i].join();
			}
			catch (InterruptedException e) {
				System.err.println("Producer join was interrupted.");
			}
		}
		//System.err.println(" Producers finished.");

		// run a producer with quit items for consumer finish
		p[0] = new Producer(storage, quitItems);
		p[0].start();
		try {
			p[0].join();
		}
		catch (InterruptedException e) {
			System.err.println("Producer join was interrupted.");
		}
		//System.err.println(" Final producer finished.");

		// wait for all consumers
		for (i = 0; i < c.length; ++i) {
			try {
				c[i].join();
			}
			catch (InterruptedException e) {
				System.err.println("Consumer join was interrupted.");
			}
		}
		//System.err.println(" Consumers finished.");

		System.out.println();
	}
}

// shared storage
class SharedStorage
{
	private char[] storage;	// shared storage
	private int in = 0;	// index of next empty space
	private int out = 0;	// index of first stored space
	private int count = 0;	// number of stored items

	// monitor: lock with condition variables
	
	// initializer
	SharedStorage(int size, char val)
	{
		storage = new char[size];
		for (int i = 0; i < storage.length; ++i)
			storage[i] = val;
	}
	
	public void append(char c) throws InterruptedException
	{
		try {
			// store item c
			storage[in] = c;
			in = (in + 1) % storage.length;
			++count;
		}
		finally {
		}
	}

	// synchronized keyword means we're using a monitor
	public char take() throws InterruptedException
	{
		try {
			// retrieve item c
			char c = storage[out];
			out = (out + 1) % storage.length;
			--count;
			return c;
		}
		finally {
		}
	}
}


// producer thread
class Producer extends Thread
{
	private final SharedStorage storage;
	private char [] items;
	
	// the shared storage is given as argument
	Producer(SharedStorage s, char [] itemsToProduce) {
		storage = s;
		items = itemsToProduce;
	}

	public void run()
	{
		Random rand = new Random();

		try {
			for (int i = 0; i < items.length; ++i) {
				// simulate production
				Thread.sleep(rand.nextInt(4)+1);

				// put item to the shared storage
				storage.append(items[i]);
			}
		}
		catch (InterruptedException e) {
			System.err.println("Producer thread was interrupted.");
		}
	}
}

// consumer thread
class Consumer extends Thread
{
	private SharedStorage storage;
	private char quitValue;

	// the shared storage is given as argument
	Consumer(SharedStorage s, char quitVal)
	{
		storage = s;
		quitValue = quitVal;
	}

	public void run()
	{
		Random rand = new Random();
		char item;

		try {
			while (true) {
				// take item from the shared storage
				item = storage.take();

				// quitValue means stop
				if (quitValue == item)
					break;

				// simulate processing
				Thread.sleep(rand.nextInt(16)+1);

				// print the result
				System.out.print(item);
			}
		}
		catch (InterruptedException e) {
			System.err.println("Consumer thread was interrupted.");
		}
	}
}

// vim:ts=4:sw=4
// EOF
