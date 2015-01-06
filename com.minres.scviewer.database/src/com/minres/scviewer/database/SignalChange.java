package com.minres.scviewer.database;

public class SignalChange implements ISignalChange {
	
	EventTime time;
	
	
	public SignalChange() {
		time=EventTime.ZERO;
	}

	public SignalChange(EventTime time) {
		super();
		this.time = time;
	}

	@Override
	public int compareTo(ISignalChange o) {
		return time.compareTo(o.getTime());
	}

	@Override
	public EventTime getTime() {
		return time;
	}

	public void setTime(EventTime time) {
		this.time = time;
	}

	@Override
	public ISignalChange duplicate() throws CloneNotSupportedException {
		return (ISignalChange) this.clone();
	}

}
