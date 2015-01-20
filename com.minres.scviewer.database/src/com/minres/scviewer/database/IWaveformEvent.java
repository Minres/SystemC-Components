package com.minres.scviewer.database;


public interface IWaveformEvent extends Comparable<IWaveformEvent>{

	public Long getTime();

	public IWaveformEvent duplicate() throws CloneNotSupportedException;

}
