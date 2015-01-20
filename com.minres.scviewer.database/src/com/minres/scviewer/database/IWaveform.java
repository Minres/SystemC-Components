package com.minres.scviewer.database;


public interface IWaveform<T extends IWaveformEvent> extends IHierNode {

	public Long getId();

	public String getKind();

	public IWaveformDb getDb();

}
