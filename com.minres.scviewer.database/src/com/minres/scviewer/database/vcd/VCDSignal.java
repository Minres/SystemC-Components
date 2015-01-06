package com.minres.scviewer.database.vcd;

import java.util.Collections;
import java.util.NavigableSet;
import java.util.TreeSet;

import com.minres.scviewer.database.EventTime;
import com.minres.scviewer.database.HierNode;
import com.minres.scviewer.database.ISignal;
import com.minres.scviewer.database.ISignalChange;
import com.minres.scviewer.database.IWaveformDb;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.SignalChange;

public class VCDSignal<T extends ISignalChange> extends HierNode implements ISignal<T> {

	private long id;

	private String fullName;

	private final String kind = "signal";
	
	private final int width;
	
	private VCDDb db;

	TreeSet<ISignalChange> values;
	
	public VCDSignal(String name) {
		this(0, name, 1);
	}

	public VCDSignal(int id, String name) {
		this(id,name,1);
	}

	public VCDSignal(int id, String name, int width) {
		super(name);
		fullName=name;
		this.id=id;
		this.width=width;
		this.values=new TreeSet<ISignalChange>();
	}

	@SuppressWarnings("unchecked")
	public VCDSignal(IWaveform other, int id, String name) {
		super(name);
		fullName=name;
		this.id=id;
		assert(other instanceof VCDSignal<?>);
		this.width=((VCDSignal<? extends ISignalChange>)other).width;
		this.values=((VCDSignal<T>)other).values;
	}

	@Override
	public String getFullName() {
		return fullName;
	}

	public void setId(int id) {
		this.id=id;
	}

	@Override
	public Long getId() {
		return id;
	}

	@Override
	public String getKind() {
		return kind;
	}

	public int getWidth() {
		return width;
	}

	@Override
	public IWaveformDb getDb() {
		return db;
	}

	public void addSignalChange(T change){
		values.add(change);
	}
	
	@Override
	public NavigableSet<ISignalChange> getSignalChanges() {
		return values;
	}

	@Override
	public T getSignalChangeByTime(EventTime time) {
		return (T) values.floor(new SignalChange(time));
	}

	@Override
	public NavigableSet<ISignalChange> getSignalChangesByTimes(EventTime start, EventTime end) {
		ISignalChange low = values.floor(new SignalChange(start));
		ISignalChange high = values.ceiling(new SignalChange(end));
		if(high!=null)
			return values.subSet(low, true, high, true);
		else
			return values.subSet(low, true, values.last(), true);
	}


}
