/*******************************************************************************
 * Copyright (c) 2015 MINRES Technologies GmbH and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     MINRES Technologies GmbH - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.database.vcd;

import java.util.NavigableMap;
import java.util.TreeMap;

import com.minres.scviewer.database.HierNode;
import com.minres.scviewer.database.ISignal;
import com.minres.scviewer.database.ISignalChange;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformDb;
import com.minres.scviewer.database.IWaveformEvent;

public class VCDSignal<T extends ISignalChange> extends HierNode implements ISignal<T> {

	private long id;

	private String fullName;

	private final String kind = "signal";
	
	private final int width;
	
	private IWaveformDb db;

	TreeMap<Long, T> values;
	
	public VCDSignal(IWaveformDb db, String name) {
		this(db, 0, name, 1);
	}

	public VCDSignal(IWaveformDb db, int id, String name) {
		this(db, id,name,1);
	}

	public VCDSignal(IWaveformDb db, int id, String name, int width) {
		super(name);
		this.db=db;
		fullName=name;
		this.id=id;
		this.width=width;
		this.values=new TreeMap<Long, T>();
	}

	@SuppressWarnings("unchecked")
	public VCDSignal(IWaveform<? extends ISignalChange> other, int id, String name) {
		super(name);
		fullName=name;
		this.id=id;
		assert(other instanceof VCDSignal<?>);
		this.width=((VCDSignal<? extends IWaveformEvent>)other).width;
		this.values=((VCDSignal<T>)other).values;
		this.db=other.getDb();
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
		values.put(change.getTime(), change);
	}
	
	@Override
	public NavigableMap<Long, T> getEvents() {
		return values;
	}

	@Override
	public T getWaveformEventsAtTime(Long time) {
		return values.get(time);
	}

    @Override
    public T getWaveformEventsBeforeTime(Long time) {
        return  values.floorEntry(time).getValue();
    }

	@Override
	public Boolean equals(IWaveform<? extends IWaveformEvent> other) {
		return(other instanceof VCDSignal<?> && this.getId()==other.getId());
	}



}
