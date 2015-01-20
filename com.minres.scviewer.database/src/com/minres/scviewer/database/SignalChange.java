package com.minres.scviewer.database;

public class SignalChange implements IWaveformEvent {
	
	Long time;
	
	
	public SignalChange() {
		time=0L;
	}

	public SignalChange(Long time) {
		super();
		this.time = time;
	}

	@Override
	public int compareTo(IWaveformEvent o) {
		return time.compareTo(o.getTime());
	}

	@Override
	public Long getTime() {
		return time;
	}

	public void setTime(Long time) {
		this.time = time;
	}

	@Override
	public IWaveformEvent duplicate() throws CloneNotSupportedException {
		return (IWaveformEvent) this.clone();
	}

}
