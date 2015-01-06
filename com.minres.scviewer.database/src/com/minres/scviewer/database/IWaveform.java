package com.minres.scviewer.database;

public interface IWaveform extends IHierNode {

	public Long getId();

	public String getKind();

	public IWaveformDb getDb();


}
