import java.io.IOException;
import java.util.*;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;

public class InvertedIndexes {

    public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, Text> {
	JobConf conf;
	public void configure( JobConf job ) {
	    this.conf = job;
	    System.err.println( "this.conf = " + job );
	}

	public void map(LongWritable docId, Text value, OutputCollector<Text, Text> output, Reporter reporter) 
	    throws IOException {
	    // retrieve # arguments from JobConf
	    int argc = Integer.parseInt( conf.get( "argc" ) );

	    // get the current file name 
	    FileSplit fileSplit = ( FileSplit )reporter.getInputSplit( );
	    String filename = "" + fileSplit.getPath( ).getName( );

	    // get prepared for a keyword-counting array
	    int[] keywordCount = new int[argc];
	    for ( int elem : keywordCount )
		elem = 0;

	    // read each line from docId
	    String line = value.toString();
	    StringTokenizer tokenizer = new StringTokenizer(line);

	    while ( tokenizer.hasMoreTokens( ) ) {
		// check if each word in this line matches any of the argc keywords
		String word = tokenizer.nextToken( );

		for ( int i = 0; i < argc; i++ ) {
		    if ( word.equals( ( String )conf.get( new String( "keyword" + i ) ) ) )
			keywordCount[i]++;
		}
	    }

	    // collect postings for each keyword
	    for ( int i = 0; i < argc; i++ ) {
		if ( keywordCount[i] > 0 ) {
		    Text word = new Text( ( String )conf.get( new String( "keyword" + i ) ) );
		    Text posting = new Text( filename + " " + keywordCount[i] );
		    output.collect( word, posting );
		}
	    }
	}
    }
	 
    public static class Reduce extends MapReduceBase implements Reducer<Text, Text, Text, Text> {
	public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output, Reporter reporter) 
	    throws IOException {

	    // prepare a posting list
	    Hashtable<String, Integer> table = new Hashtable<String, Integer>( );

	    while ( values.hasNext( ) ) {
			// retrieve a posting to be inserted
			Text newPosting = values.next( );
			StringTokenizer newPosting_tokenizer = new StringTokenizer( newPosting.toString( ) );
			if ( !newPosting_tokenizer.hasMoreTokens( ) )
				continue;
			String new_filename = newPosting_tokenizer.nextToken( ); 
			if ( !newPosting_tokenizer.hasMoreTokens( ) )
				continue;
			int new_count = Integer.parseInt( newPosting_tokenizer.nextToken( ) );

			Integer prev_count = table.get( new_filename );
			if ( prev_count != null )
				table.put( new_filename, Integer.valueOf( prev_count.intValue( ) + new_count ) );
			else
				table.put (new_filename, Integer.valueOf( new_count ) );
	    }

	    // concatinate all postings in one string
	    String docList = "";
	    for ( Enumeration<String> filenames =  table.keys( ); filenames.hasMoreElements( ); ) {
		String filename = filenames.nextElement( );
		String count = ( table.get( filename ) ).toString( );
		docList += filename + " " + count + " ";
	    }
	    Text docListText = new Text( docList );

	    // print it out.
	    output.collect(key, docListText );
	}
    }
    
    public static void main(String[] args) throws Exception {
	// input format:
	// hadoop jar invertedindexes.jar InvertedIndexes rfc output keyword1 keyword2 keyword3 keyword3 keyword4

	JobConf conf = new JobConf(InvertedIndexes.class);
	conf.setJobName("invertedindexes");
	
	conf.setOutputKeyClass(Text.class);
	conf.setOutputValueClass(Text.class);

	conf.setJar("invertedindexes.jar"); 
	conf.setMapperClass(InvertedIndexes.Map.class);
	conf.setCombinerClass(InvertedIndexes.Reduce.class);
	conf.setReducerClass(InvertedIndexes.Reduce.class);
	
	conf.setInputFormat(TextInputFormat.class);
	conf.setOutputFormat(TextOutputFormat.class);
	
	FileInputFormat.setInputPaths(conf, new Path(args[0]));
	FileOutputFormat.setOutputPath(conf, new Path(args[1]));

	conf.set( "argc", String.valueOf( args.length - 2 ) );
	for ( int i = 0; i < args.length - 2; i++ )
	    conf.set( "keyword" + i, args[i + 2] );
	
	Date startTime = new Date( );
	RunningJob job = JobClient.runJob(conf);
	job.waitForCompletion( );
	Date endTime = new Date( );
	System.out.println( "Elapsed time = " + ( endTime.getTime( ) - startTime.getTime( ) ) );
    }
}
