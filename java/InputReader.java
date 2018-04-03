import java.util.Scanner;
import java.io.File;
import java.io.FileNotFoundException;

public class InputReader {

	private Scanner input;

	public InputReader(String filename) {
		try {
			input = new Scanner(new File(filename));
		}
		catch (FileNotFoundException fnfe) {
			System.out.println(fnfe);
			System.exit(-1);
		}
	}

	public InputReader() {
		this("prog2.input");
	}

	public void read() {
		while(input.hasNextLine()) {
			String nextLine = input.nextLine();

			int numProcesses = -1;
			int numResources = -1;

			if (nextLine.matches("^\\d+\\s\\d+$")) {
				numProcesses = Character.getNumericValue(nextLine.charAt(0));
				numResources = Character.getNumericValue(nextLine.charAt(2));
				System.out.printf("np = %d --- nr = %d\n", numProcesses, numResources);
			}

			if (numProcesses == 0 && numResources == 0) {
				System.out.println("File reading complete.");
				return;
			}	
		}
	}


}