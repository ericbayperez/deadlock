import java.util.ArrayList;

public class DeadlockProblem {
    
    private int numProcesses;
    private int numResources;
    ArrayList<Process> processes;

    public DeadlockProblem(int np, int nr, ArrayList<Process> plist) {
        numProcesses = np;
        numResources = nr;
        processes = plist;
    }

    @Override
    public String toString() {
        String s = "=== PROBLEM ===\n";
        for (Process p : processes) {
            s += String.format("%s\n", p);
        }
        return s;
    }

}