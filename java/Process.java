import java.util.ArrayList;

public class Process {

	private Action[] actions;
	private int nextActionIndex;
	private int runtime;
	private int endtime;

	public Process(Action[] a) {
		actions = a;
	}

	@Override
	public String toString() {
		String s = "[Process]\n";
		for (Action a : actions) {
			s += String.format("    %s\n", a);
		}
		return s;
	}
}