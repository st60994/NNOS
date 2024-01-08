// Operating Systems: sample code  (c) Tomáš Hudec
// Threads, Java
// Synchronization
//
// Synchronization SOLVED using Java Monitor
//
// Last modification: 2015-11-30

/* Zadání

Vyřešte problém synchronizace
bez aktivního čekání
užitím monitoru jazyka Java.

Program musí vypsat čísla v pořadí 1, 2 nezávisle na zadané hodnotě čekání sleep.
Řešení musí být stále funkční, i když se vloží sleep kamkoliv.

Použijte vhodné komentáře.

Výsledný program pojmenujte Sync.java a vytvořte Makefile.

*/

/* Assignment

Solve the synchronization problem
with non-busy wait
using Java monitor.

The program must print numbers in order 1, 2 independently on sleep value.
The solution must be still functional even if the sleep statement
is put anywhere in the source.

Use suitable comments.

Compilation:
  javac SyncMonitor.java
Run:
  java SyncMonitor

*/

import java.util.Random;

public class SyncMonitor
{
	public static void main(String [] args)
	{
		int SleepVal = 0;	// milliseconds
	
		// parse options
		// the program expects one number: sleep value
		switch (args.length) {
			case 1:
				SleepVal = Integer.parseInt(args[0]);
				if (SleepVal > 2000)
					SleepVal = 2000;
				if (SleepVal < 0)
					SleepVal = 0;
			case 0:
				break;
			default:
				System.err.println("One argument is expected – sleep in milliseconds.");
				System.exit(1);
		}

		int Value = 1;

		// initialize shared storage buffer
		Calculation c = new Calculation(Value);
		
		// initialize a thread
		PrintThread t = new PrintThread(c);

		// start the thread
		//System.err.println("Starting thread.");
		t.start();

		// sleep
		try { Thread.sleep(SleepVal); }
		catch (InterruptedException e) { }

		// this must be run first
		System.out.println("1: doing calculation, initial value is " + Value);

		// change the value
		c.CalculateValue();

		// wait for the thread
		try { t.join(); }
		catch (InterruptedException e) {
			e.printStackTrace();
		}

		//System.err.println("Threads finished.");
	}
}

class Calculation
{
	// all variables in a monitor must be private
	private int val = 0;
	private Boolean calculated = false;	// for synchronization
	
	// initializer
	Calculation(int v)
	{
		val = v;
	}
	
	// synchronized keyword means we're using a monitor
	public synchronized void CalculateValue()
	{
		val++;

		// signalize state change: calculation is done
		calculated = true;
		notify();
	}

	// synchronized keyword means we're using a monitor
	public synchronized int GetValue()
	{
		// wait while the calculation is not done
		while (!calculated)
		{
			try { wait(); }	// non-busy wait using monitor
			catch (InterruptedException e) { }
			finally { }
		}

		return val;
	}
}

/*
 * Thread 1
 */
class PrintThread extends Thread
{
	private Calculation c;
	
	PrintThread(Calculation initc) {
		c = initc;
	}

	public void run()
	{
		// sleep
		try { Thread.sleep(0); }
		catch (InterruptedException e) { }

		// this must be run second
		int Value = c.GetValue();
		System.out.println("2: using calculated value " + Value);
	}
}    

