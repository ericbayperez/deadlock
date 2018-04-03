import java.util.ArrayList;

public class Process {

	private int numActions;
	private Action[] actions;
	private int[] parameters;
	private int nextActionIndex;
	private int runtime;
	private int endtime;

	public Process(Action[] a, int[] p) {
		actions = a;
		parameters = p;
	}
}