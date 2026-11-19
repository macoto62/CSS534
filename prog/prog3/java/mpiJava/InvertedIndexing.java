import mpi.*;           // for mpiJava
import java.util.*;     // for Date, Scanner
import java.io.*;       // for FileInputStream

public class InvertedIndexing {
    // mpi-related values
    static int myrank = 0;;
    static int nprocs = 0;
    final static int master = 0;

    /**
     * Is the constructor that implements inverted indexing.
     *
     * @param inputPath the path to an input directory
     * @param keywords  an array of keywords to look for
     * @param filenames a list of all file names in the given input directory
     */
    public InvertedIndexing( String inputPath, String[] keywords, String[] filenames ) throws MPIException {
	String[] emits = new String[keywords.length]; // outputs are emitted here.

	// map
	for ( int i = myrank; i < filenames.length; i += nprocs ) { // each rank takes care of filename[rank + i];
	    int[] keywordCount = new int[keywords.length];          // the occurence of keywords initialize
	    for ( int elem : keywordCount )
		elem = 0;

	    // read word by word from a filename[rank + i] 
	    try {
		Scanner file = new Scanner( new FileInputStream( inputPath + "/" + filenames[i] ) );
		while ( file.hasNext( ) ) {
		    String word = file.next( );
		    
		    for ( int j = 0; j < keywords.length; j++ ) {
			if ( word.equals( keywords[j] ) ) // check this word hits keyword[j]
			    keywordCount[j]++;            // then increment its count
		    }
		}
	    } catch ( Exception e ) {
		e.printStackTrace( );
	    }

	    // create a list of tuplets <filename occurent> for each keyword[j]
	    for ( int j = 0; j < keywords.length; j++ ) {
		if ( keywordCount[j] > 0 ) 
		    emits[j] += ( " " + filenames[i] + " " + keywordCount[j] ); 
	    }
	}

	// reduce
	for ( int j = 0; j < keywords.length; j++ ) { // for each keyword, collect a list of <filename occurence>
	    if ( myrank == 0 ) {
		for ( int worker = 1; worker < nprocs; worker++ ) {
		    Object[] recvObject = new Object[1];
		    MPI.COMM_WORLD.Recv( recvObject, 0, 1, MPI.OBJECT, worker, 0 );
		    if ( recvObject[0] != null )
			emits[j] += ( String )recvObject[0];
		}
		System.out.println( keywords[j] + "\t" + emits[j] ); // print this keyword's list of <filename occurence>	
	    }
	    else {
		MPI.COMM_WORLD.Send( emits, j, 1, MPI.OBJECT, master, 0 );
	    }
	}
    }

    /**
     * Usage: java_mpirun.sh N InvertedIndexing input_path keywords_path
     *
     * @param args Receive an input directory path in args[0] and a file name of keywords in args[1]
     */
    public static void main( String[] args ) throws MPIException {

	// Check # args.
	if ( args.length != 2 ) { 
	    System.out.println( "usage: " +
				"java_mpirun.sh 1 InvertedIndexing input_path keywords_path" );
	    System.exit( -1 );
	}

	// Start the MPI library.
	MPI.Init( args );
	myrank = MPI.COMM_WORLD.Rank( );
	nprocs = MPI.COMM_WORLD.Size( );

	// initialize inputPath and keywordPaths.
	String inputPath = args[0];
	String keywordsPath = args[1];
	String keywords[] = getKeywords( keywordsPath );	

	// debugging
	if ( myrank == 0 ) {
	    System.out.println( "rank[" + myrank + "]: input_path = " + inputPath +
				"; keywords_path = " + keywordsPath );
	    for ( String key : keywords )
		System.out.println( "rank[" + myrank + "]: keyword = " + key );
	}
	    
	// Start Computation
	Date startTime = new Date( );

	// Read all file names
	String[] filenames = getFileNames( inputPath );
	
	// An actual implementation of inverted indexing
	new InvertedIndexing( inputPath, keywords, filenames );

	// Terminate the MPI library.
	if ( myrank == 0 ) {
	    Date endTime = new Date( );
	    System.out.println( "Elapsed time = " + ( endTime.getTime( ) - startTime.getTime( ) ) );
	}
	MPI.Finalize( );
    }

    /**
     * Retrieves all keywords.
     *
     * @param path the path to a file that include all keywords to search 
     * @return a list of keywords
     */
    private static String[] getKeywords( String path ) {
	String keywords[] = new String[5]; // We assume only five keywords

	// retrieve up to five keywords from  a given keyword file
	try { 
	    Scanner keywordsFile = new Scanner(  new FileInputStream( path ) );
	    for ( int i = 0; keywordsFile.hasNext( ) && i < 5; i++ )
		keywords[i] = keywordsFile.next( );
	} catch ( Exception e ) {
	    e.printStackTrace( );
	}
	return keywords;
    }

    /**
     * Retrieves all file names to look for.
     *
     * @param path the path to a directory that includes all files to read
     * @return a list of file names
     */
    private static String[] getFileNames( String path ) {
	String filenames[] = null;

	// retrieve all file names from a given directory in path.
	try {
	    File dir = new File( path );
	    if ( dir.exists() && dir.isDirectory( ) ) {     // only if the path is a directory
		File[] files = dir.listFiles( );            // a list of all files
		if ( files != null ) {
		    filenames = new String[ files.length ];
		    for (int i = 0; i < filenames.length; i++ )
			filenames[i] = files[i].getName( ); // retrieve each file name.
		}
		
	    }
	} catch ( Exception e ) {
	    e.printStackTrace( );
	}
	return filenames;
    }
}
