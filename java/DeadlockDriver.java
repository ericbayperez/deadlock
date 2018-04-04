import java.util.Scanner;
import java.io.File;
import java.io.FileNotFoundException;


import java.util.ArrayList;

public class DeadlockDriver {


	public static void main(String[] args) {
		Scanner input = null;
		
		/* arguments from command line */
		boolean verbose = false;
		String filename = "";


		if (args.length == 1) {
			filename = args[0];
		}
		else if (args.length == 2 && args[0].equals("-v")) {
			verbose = true;
			filename = args[1];
		}
		else {
			System.out.println("Usage: ./prog [-v] [filename]");
			System.exit(-1);
		}

		InputReader reader = new InputReader(filename);
		ArrayList<DeadlockProblem> problemList = reader.read();

		for (DeadlockProblem dp : problemList) {
			System.out.println(dp);
		}
		
		System.out.println("End of main.");

	}


}