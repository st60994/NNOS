// Operating Systems: sample code  (c) Tomáš Hudec
// Threads, Java
// Critical Sections, Synchronization
//
// Producer/Consumer Problem TO BE SOLVED
//
// Last modification: 2015-11-30

/* Zadání

Vyřešte problém konzumentů a producentů
bez aktivního čekání
užitím monitoru jazyka Java.

Problémy:
-- synchronizace při prázdném a plném skladu,
-- kritická sekce při vstupu do skladu.

Ošetřete chybové stavy hláškou o chybě a ukončením programu s návratovým
kódem různým od nuly.

Použijte vhodné komentáře.

Výsledný program pojmenujte ProducerConsumerMonitor.java a vytvořte Makefile.

*/

/* Assignment

Solve the producer-consumer problem
with non-busy wait
using Java monitor.

Problems:
-- synchronization if the shared storage buffer is empty and full
-- race condition while entering the storage buffer

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
		int TotalProducers  = 2;	// number of producers
		int TotalConsumers  = 1; 	// number of consumers
		int StorageCapacity = 5;	// storage capacity
		char [] [] toBeProducedData = {
			"abcdefghijklmnopqrstuvwxyz".toCharArray(),
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ".toCharArray(),
			"0123456789".toCharArray(),
		};
		char [] QuitItems;
		char QuitItem = '^';	// upon reading this item the consumer ends
		char EmptyItem = '_';	// free place in a storage buffer
		int i;
	
		// parse options
		// the program expects three numbers:
		//	1st number of producers
		//	2nd number of consumers
		//	3rd storage capacity
		switch (args.length) {
			case 3:
				StorageCapacity = Integer.parseInt(args[2]);
				if (StorageCapacity > 10)
					StorageCapacity = 10;
				if (StorageCapacity < 1)
					StorageCapacity = 1;
			case 2:
				TotalConsumers = Integer.parseInt(args[1]);
				if (TotalConsumers > 50)
					TotalConsumers = 50;
				if (TotalConsumers < 1)
					TotalConsumers = 1;
			case 1:
				TotalProducers = Integer.parseInt(args[0]);
				if (TotalProducers > 3)
					TotalProducers = 3;
				if (TotalProducers < 1)
					TotalProducers = 1;
			case 0:
				break;
			default:
				System.exit(1);
		}

		// initialize shared storage buffer
		SharedStorage buf = new SharedStorage(StorageCapacity, EmptyItem);
		
		// initialize number of producers
		Producer [] p = new Producer[TotalProducers];
		// initialize number of consumers
		Consumer [] c = new Consumer[TotalConsumers];
		// for quitting
		QuitItems = new char[c.length];

		// start consumers
		//System.err.println("Starting " + c.length + " consumer(s).");
	       	for (i = 0; i < c.length; i++) {
			c[i] = new Consumer(buf, QuitItem);
			c[i].start();
			QuitItems[i] = QuitItem; // fill the array for final producer
		}

		// start producers
		//System.err.println("Starting " + p.length + " producer(s).");
	       	for (i = 0; i < p.length; i++) {
			p[i] = new Producer(buf, toBeProducedData[i]);
			p[i].start();
		}

		// wait for all producers
	       	for (i = 0; i < p.length; i++) {
			try {
				p[i].join();
			}
			catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		//System.err.println(" Producers finished.");

		// run a producer with quit items for consumer finish
		p[0] = new Producer(buf, QuitItems);
		p[0].start();
		try { p[0].join(); }
		catch (InterruptedException e) {
			e.printStackTrace();
		}
		//System.err.println(" Final producer finished.");

		// wait for all consumers
	       	for (i = 0; i < c.length; i++) {
			try {
				c[i].join();
			}
			catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		//System.err.println(" Consumers finished.");

		System.out.println();
	}
}

class SharedStorage
{
	private char [] buffer;	// shared storage buffer
	private int in = 0;	// index of next empty space
	private int out = 0;	// index of first stored space
	private int count = 0;	// number of stored items
	
	// initializer
	SharedStorage(int size, char val)
	{
		buffer = new char[size];
		for (int i = 0; i < buffer.length; i++)
			buffer[i] = val;
	}
	
	public void Append(char c)
	{
		// wait if the buffer is full
		//while (count == buffer.length)
			//;
		
		// store item c
		buffer[in] = c;
		in = (in + 1) % buffer.length;
		count++;

		// signalize state change: non-empty storage
	}

	public char Take()
	{
		// wait if the buffer is empty

		// retrieve item c
		char c = buffer[out];
		out = (out + 1) % buffer.length;
		count--;

		// 

		return c;
	}
}


/*
 * Producer
 */
class Producer extends Thread
{
	private SharedStorage buffer;
	private char [] items;
	
	// shared storage buffer is given as argument
	Producer(SharedStorage b, char [] itemsToProduce) {
		buffer = b;
		items = itemsToProduce;
	}

	public void run()
	{
		Random rand = new Random();

		for (int i = 0; i < items.length; i++) {
			// simulate production
			try { Thread.sleep(rand.nextInt(4)+1); }
			catch (InterruptedException e) { }

			// put item to shared storage buffer
			buffer.Append(items[i]);
		}
	}
}    

/*
 * Consumer
 */
class Consumer extends Thread
{
	private SharedStorage buffer;
	private char QuitValue;
   
	// shared storage buffer is given as argument
	Consumer(SharedStorage b, char quitVal)
	{
		buffer = b;
		QuitValue = quitVal;
	}

	public void run()
	{
		Random rand = new Random();
		char item;

		for (;;) {
			// take item from shared storage buffer
			item = buffer.Take();

			// QuitValue means stop
			if (item == QuitValue)
				break;

			// simulate processing
			try { Thread.sleep(rand.nextInt(16)+1); }
			catch (InterruptedException e) { }

			// print the result
			System.out.print(item);
		}
	}
}   


