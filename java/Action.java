public class Action {
	
	public char type;
	public int value;

	public Action() {
		type = ' ';
		value = -1;
	}

	public Action(char t, int v) {
		type = t;
		value = v;
	}

	@Override
	public String toString() {
		return String.format("[Action] %c%d", type, value);
	}
}