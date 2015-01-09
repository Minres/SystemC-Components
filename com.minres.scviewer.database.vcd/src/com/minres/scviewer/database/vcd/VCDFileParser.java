package com.minres.scviewer.database.vcd;

import java.io.*;
import java.util.*;

import com.minres.scviewer.database.BitVector;

class VCDFileParser {
	private StreamTokenizer tokenizer;
	private IVCDDatabaseBuilder traceBuilder;
	private HashMap<String, Integer> nameToNetMap = new HashMap<String, Integer>();
	private long picoSecondsPerIncrement;
	private boolean stripNetWidth;
	long currentTime;

	public VCDFileParser(boolean stripNetWidth) {
		this.stripNetWidth=stripNetWidth;
	}

	public boolean load(InputStream is, IVCDDatabaseBuilder builder) {
		tokenizer = new StreamTokenizer(new BufferedReader(new InputStreamReader(is)));
		tokenizer.resetSyntax();
		tokenizer.wordChars(33, 126);
		tokenizer.whitespaceChars('\r', '\r');
		tokenizer.whitespaceChars('\n', '\n');
		tokenizer.whitespaceChars(' ', ' ');
		tokenizer.whitespaceChars('\t', '\t');
		try {
			traceBuilder = builder;
			currentTime=0;
			while (parseDefinition());
			while (parseTransition());
			return true;
		} catch (Exception exc) {
			exc.printStackTrace();
			return false;
		}
	}

	private void parseScope() throws Exception {
		nextToken(); // Scope type (ignore)
		nextToken();
		traceBuilder.enterModule(tokenizer.sval);
		match("$end");
	}

	private void parseUpscope() throws Exception {
		match("$end");
		traceBuilder.exitModule();
	}

	private void parseVar() throws Exception {
		nextToken(); // type
		nextToken(); // size
		int width = Integer.parseInt(tokenizer.sval);

		nextToken();
		String id = tokenizer.sval;
		nextToken();
		String netName = tokenizer.sval;
		while (nextToken() && !tokenizer.sval.equals("$end")) {
			netName+=tokenizer.sval;
		}

		Integer net = nameToNetMap.get(id);
		if (net == null) {
			// We've never seen this net before
			if(stripNetWidth){
				int openBracket = netName.indexOf('[');
				if (openBracket != -1) netName = netName.substring(0, openBracket);
			}
			nameToNetMap.put(id, traceBuilder.newNet(netName, -1, width));
		} else {
			// Shares data with existing net. Add as clone.
			traceBuilder.newNet(netName, net, width);
		}
	}

	private void parseTimescale() throws Exception {
		nextToken();
		String s = tokenizer.sval;
		nextToken();
		while(!tokenizer.sval.equals("$end")){
			s+=" "+tokenizer.sval;
			nextToken();
		}
		switch (s.charAt(s.length() - 2)){
		case 'p': // Nano-seconds
			picoSecondsPerIncrement = 1;
			s = s.substring(0, s.length() - 2).trim();
			break;
		case 'n': // Nano-seconds
			picoSecondsPerIncrement = 1000;
			s = s.substring(0, s.length() - 2).trim();
			break;
		case 'u': // Microseconds
			picoSecondsPerIncrement = 1000000;
			s = s.substring(0, s.length() - 2).trim();
			break;
		case 'm': // Microseconds
			picoSecondsPerIncrement = 1000000000;
			s = s.substring(0, s.length() - 2).trim();
			break;
		default: // Seconds
			picoSecondsPerIncrement = 1000000000000L;
			s = s.substring(0, s.length() - 1);
			break;
		}
		picoSecondsPerIncrement *= Long.parseLong(s);
	}

	private boolean parseDefinition() throws Exception {
		nextToken();
		if (tokenizer.sval.equals("$scope"))
			parseScope();
		else if (tokenizer.sval.equals("$var"))
			parseVar();
		else if (tokenizer.sval.equals("$upscope"))
			parseUpscope();
		else if (tokenizer.sval.equals("$timescale"))
			parseTimescale();
		else if (tokenizer.sval.equals("$enddefinitions")) {
			match("$end");
			return false;
		} else {
			// Ignore this defintion
			do {
				if (!nextToken()) return false;
			} while (!tokenizer.sval.equals("$end"));
		}

		return true;
	}

	private boolean parseTransition() throws Exception {
		if (!nextToken()) return false;
		if (tokenizer.sval.charAt(0) == '#') {	// If the line begins with a #, this is a timestamp.
			currentTime = Long.parseLong(tokenizer.sval.substring(1)) * picoSecondsPerIncrement;
		} else {
			if(tokenizer.sval.equals("$comment")){
				do {
					if (!nextToken()) return false;
				} while (!tokenizer.sval.equals("$end"));
				return true;
			}
			if (tokenizer.sval.equals("$dumpvars") || tokenizer.sval.equals("$end")) return true;
			String value, id;
			if (tokenizer.sval.charAt(0) == 'b') {
				// Multiple value net. Value appears first, followed by space,
				// then identifier
				value = tokenizer.sval.substring(1);
				nextToken();
				id = tokenizer.sval;
			} else {
				// Single value net. identifier first, then value, no space.
				value = tokenizer.sval.substring(0, 1);
				id = tokenizer.sval.substring(1);
			}

			Integer net = nameToNetMap.get(id);
			if (net == null) {
				System.out.println("unknown net " + id + " value " + value);
				return true;
			}

			int netWidth = traceBuilder.getNetWidth(net);
			BitVector decodedValues = new BitVector(netWidth);
			if (value.equals("z") && netWidth > 1) {
				for (int i = 0; i < netWidth; i++)
					decodedValues.setValue(i, BitVector.VALUE_Z);
			} else if (value.equals("x") && netWidth > 1) {
				for (int i = 0; i < netWidth; i++)
					decodedValues.setValue(i, BitVector.VALUE_X);
			} else {
				int stringIndex = 0;
				for (int convertedIndex = netWidth - value.length(); convertedIndex < netWidth; convertedIndex++) {
					switch (value.charAt(stringIndex++)) {
					case 'z':
						decodedValues.setValue(convertedIndex, BitVector.VALUE_Z);
						break;

					case '1':
						decodedValues.setValue(convertedIndex, BitVector.VALUE_1);
						break;

					case '0':
						decodedValues.setValue(convertedIndex, BitVector.VALUE_0);
						break;

					case 'x':
						decodedValues.setValue(convertedIndex, BitVector.VALUE_X);
						break;

					default:
						decodedValues.setValue(convertedIndex, BitVector.VALUE_X);
					}
				}
			}
			traceBuilder.appendTransition(net, currentTime, decodedValues);
		}
		return true;
	}

	private void match(String value) throws Exception {
		nextToken();
		if (!tokenizer.sval.equals(value)) 
			throw new Exception("Line "+tokenizer.lineno()+": parse error, expected "+value+" got "+tokenizer.sval);
	}

	private boolean nextToken() throws IOException {
		return tokenizer.nextToken() != StreamTokenizer.TT_EOF;
	}

};
