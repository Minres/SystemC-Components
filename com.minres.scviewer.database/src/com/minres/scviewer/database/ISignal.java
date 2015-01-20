package com.minres.scviewer.database;

import java.util.NavigableMap;


public interface ISignal<T extends ISignalChange> extends IWaveform<T>{

	public NavigableMap<Long, T> getEvents();

	public T getWaveformEventsAtTime(Long time);


}

