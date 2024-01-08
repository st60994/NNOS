// Operating Systems: sample code  (c) Microsoft, Tomáš Hudec
// Threads, C#
// Critical Sections, Synchronization
//
// simplified Producer/Consumer Problem and Monitor
//
// Last modification: 2015-11-30
//
// URL of the original: http://msdn.microsoft.com/en-us/library/aa645740%28v=vs.71%29.aspx

// Monitor class is used
// for critical section access control:		lock(Object)
// for synchronization:				Monitor.Pulse(Object) and Monitor.Wait(Object)

// use Mono to compile:
//	mcs ProducerConsumerSimple.cs

using System;
using System.Threading;

public class ProducerConsumer
{
	public static void Main(String[] args)
	{
		int result = 0;				// result initialized to say there is no error
		Cell cell = new Cell();			// storage buffer

		CellProd prod = new CellProd(cell, 20);	// use cell for storage, produce 20 items
		CellCons cons = new CellCons(cell, 20);	// use cell for storage, consume 20 items

		// create producer and consumer threads
		Thread producer = new Thread(new ThreadStart(prod.ThreadRun));
		Thread consumer = new Thread(new ThreadStart(cons.ThreadRun));

		try {
			// run both threads until done
			producer.Start();
			consumer.Start();

			// join both threads with no timeout
			producer.Join();
			consumer.Join();  
		}
		catch (ThreadStateException e) {
			Console.WriteLine(e);		// display text of an exception
			result = 1;			// result says there was an error
		}
		catch (ThreadInterruptedException e) {
			Console.WriteLine(e);		// this exception means that the thread was interrupted during a Wait
			result = 1;			// result says there was an error
		}

		// even though Main returns void, this provides a return code to the parent process
		Environment.ExitCode = result;
	}
}

// producer thread
public class CellProd
{
	Cell cell;					// field to hold cell object to be used (storage)
	int quantity = 1;				// field for how many items to produce in cell

	public CellProd(Cell box, int request)
	{
		cell = box;				// pass in what cell object to be used
		quantity = request;			// pass in how many items to produce in cell
	}

	public void ThreadRun()
	{
		for (int looper = 1; looper <= quantity; ++looper)
			cell.WriteToCell(looper);	// producing
	}
}

// consumer thread
public class CellCons
{
	Cell cell;					// field to hold cell object to be used (storage)
	int quantity = 1;				// field for how many items to consume from cell

	public CellCons(Cell box, int request)
	{
		cell = box;				// pass in what cell object to be used
		quantity = request;			// pass in how many items to consume from cell
	}

	public void ThreadRun()
	{
		int valReturned;
		for (int looper = 1; looper <= quantity; ++looper)
			// consume the result by placing it in valReturned.
			valReturned = cell.ReadFromCell();
	}
}

// storage cell
public class Cell
{
	int cellContents;				// cell contents
	bool readerFlag = false;			// state flag

	public int ReadFromCell()
	{
		lock(this)				// enter synchronization block
		{
			while (!readerFlag) {		// wait until Cell.WriteToCell is done producing
				try {			// waits for the Monitor.Pulse in WriteToCell
					Monitor.Wait(this);
				}
				catch (SynchronizationLockException e) {
					Console.WriteLine(e);
				}
				catch (ThreadInterruptedException e) {
					Console.WriteLine(e);
				}
			}
			Console.WriteLine("Consume: {0}", cellContents);
			readerFlag = false;		// reset the state flag to say consuming is done
			Monitor.Pulse(this);		// pulse tells Cell.WriteToCell that Cell.ReadFromCell is done
		}					// exit synchronization block
		return cellContents;
	}

	public void WriteToCell(int n)
	{
		lock(this)				// enter synchronization block
		{
			while (readerFlag) {		// wait until Cell.ReadFromCell is done consuming
				try {			// wait for the Monitor.Pulse in ReadFromCell
					Monitor.Wait(this);
				}
				catch (SynchronizationLockException e) {
					Console.WriteLine(e);
				}
				catch (ThreadInterruptedException e) {
					Console.WriteLine(e);
				}
			}
			cellContents = n;
			Console.WriteLine("Produce: {0}", cellContents);
			readerFlag = true;		// reset the state flag to say producing is done
			Monitor.Pulse(this);		// pulse tells Cell.ReadFromCell that Cell.WriteToCell is done
		}					// exit synchronization block
	}
}
