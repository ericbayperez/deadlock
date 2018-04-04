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
		// Until the end of the file is reached...
		while(input.hasNextLine()) {
			String nextLine = input.nextLine();

			int numProcesses = -1;
			int numResources = -1;

			// If a new set of problems is starting...
			if (nextLine.matches("^\\d+\\s\\d+$")) {
				String[] strNums = nextLine.split(" ");
				numProcesses = Integer.parseInt(strNums[0]);
				numResources = Integer.parseInt(strNums[1]);
				System.out.printf("np = %d --- nr = %d\n", numProcesses, numResources);
			}

			// Otherwise if the next line is a process...
			else {
				String[] strActions = nextLine.split("\\s+");

				// First value on this line is a number stating the
				// number of actions for the process on this line
				int numActions = Integer.parseInt(strActions[0]);

				// Create an array to store these actions
				Action[] actionList = new Action[numActions];

				// For each action on this line...
				for (int i = 1; i < strActions.length; i++) {
					String strAction = strActions[i];

					// Parse the string to create an Action
					Action a = actionFromString(strAction);
					if (a != null) {
						// Add this action to the action list
						actionList[i-1] = a;
					}
					// If the string was not in the correct format, quit reading
					else {
						System.out.printf("ERROR: Problem reading the action [%s].\n", strAction);
						System.exit(-1);
					}

					// Create a new process to store the list of actions
					Process p = new Process(actionList);
				}
			}

			if (numProcesses == 0 && numResources == 0) {
				System.out.println("File reading complete.");
				break;
			}	
		}
	}

	private Action actionFromString(String strAction) {
		Action a = null;
		if (strAction.length() >= 2) {
			a = new Action();
			a.type = strAction.charAt(0);
			a.value = Integer.parseInt(strAction.substring(1));
		}
		return a;
	}
	
}