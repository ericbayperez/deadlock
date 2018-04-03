public class Deadlock {

	public static final int MAXSTEP = 50;   /* max action steps for any process */
	public static final int MAXPROC = 50;   /* max processes in any simulation  */
	public static final int MAXRSRC = 50;   /* max resources in any simulation  */

	private int numProcesses;
	private int numResources;
	private Process[] processes;
	private Node[] prn;

	public Deadlock(int np, int nr, Process[] proc) {
		numProcesses = np;
		numResources = nr;
		processes = proc;
		prn = new Node[MAXPROC + MAXRSRC];
	}
}