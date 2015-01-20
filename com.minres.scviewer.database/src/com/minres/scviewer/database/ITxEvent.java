package com.minres.scviewer.database;

public interface ITxEvent extends IWaveformEvent {
	enum Type {BEGIN, END};
	
	public ITx getTransaction();
	
	public Type getType();
}
