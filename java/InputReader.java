import java.util.Scanner;
import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;

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

	public ArrayList<DeadlockProblem> read() {

		ArrayList<Process> processList = new ArrayList<>();
		ArrayList<DeadlockProblem> problemList = new ArrayList<>();

		// Until the end of the file is reached...
		while(input.hasNextLine()) {
			String nextLine = input.nextLine();

			int numProcesses = -1;
			int numResources = -1;

			// If a new set of problems is starting...
			if (nextLine.matches("^\\d+\\s\\d+$")) {

				// If we have processes that we need to store...
				if (!processList.isEmpty()) {
					// Create a new deadlock problem from this process list
					DeadlockProblem dp = new DeadlockProblem(numProcesses, numResources, processList);
					// Add this problem to the list of deadlock problems
					problemList.add(dp);

					// Clear the current process list
					processList = new ArrayList<>();
				}
				// Parse the line to determine the numProcesses and numResources
				String[] strNums = nextLine.split(" ");
				numProcesses = Integer.parseInt(strNums[0]);
				numResources = Integer.parseInt(strNums[1]);

				// DEBUG
				System.out.printf("np = %d --- nr = %d\n", numProcesses, numResources);
			}

			// Otherwise if the next line is a process...
			else {
				// Split the string into action strings (space delimited)
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

					// Add this process to the list of processes
					processList.add(p);
				}
			}

			if (numProcesses == 0 && numResources == 0) {
				System.out.println("File reading complete.");
				break;
			}	
		}
		
		return problemList;
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