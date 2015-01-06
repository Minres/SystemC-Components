package com.minres.scviewer.database;

import java.util.NavigableSet;

public interface ISignal<T extends ISignalChange> extends IWaveform{

	public NavigableSet<ISignalChange> getSignalChanges();

	public T getSignalChangeByTime(EventTime time);

	public NavigableSet<ISignalChange> getSignalChangesByTimes(EventTime start, EventTime end);

}
