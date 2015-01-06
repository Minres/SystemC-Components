package com.minres.scviewer.database;

public interface ISignalChange extends Comparable<ISignalChange>{

	public EventTime getTime();

	public ISignalChange duplicate() throws CloneNotSupportedException;

}
