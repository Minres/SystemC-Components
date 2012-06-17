package com.itjw.txviewer.database;

import java.io.InputStream;
import java.util.List;


public interface ITrDb extends ITrHierNode {

	public EventTime getMaxTime();
	
	public List<ITrStream> getAllStreams();
	
	public void load(InputStream inp) throws InputFormatException;

	public void clear();

}
