/*******************************************************************************
 * Copyright (c) 2012 IT Just working.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     IT Just working - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.database.text

import java.beans.PropertyChangeListener;
import java.util.Collection;
import java.util.Comparator;
import java.util.List;
import java.util.Map;
import java.util.NavigableMap;

import com.google.common.collect.TreeMultimap
import com.minres.scviewer.database.ITxEvent;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformDb
import com.minres.scviewer.database.IWaveformEvent
import com.minres.scviewer.database.ITxGenerator
import com.minres.scviewer.database.HierNode;
import com.minres.scviewer.database.IHierNode
import com.minres.scviewer.database.ITxStream
import com.minres.scviewer.database.ITx

class TxStream extends HierNode implements ITxStream {

	Long id
	
	IWaveformDb database
	
	String fullName
	
	String kind
	
	def generators = []
	
	int maxConcurrency
	
	private TreeMap<Long, List<ITxEvent>> events
	
	TxStream(IWaveformDb db, int id, String name, String kind){
		super(name)
		this.id=id
		this.database=db
		this.fullName=name
		this.kind=kind
		this.maxConcurrency=0
		events = new TreeMap<Long, List<ITxEvent>>()
	}

	List<ITxGenerator> getGenerators(){
		return generators as List<ITxGenerator>
	}

	@Override
	public IWaveformDb getDb() {
		return database
	}

	@Override
	public int getMaxConcurrency() {
		if(!maxConcurrency){
			generators.each {TxGenerator generator ->
				generator.transactions.each{ Tx tx ->
					putEvent(new TxEvent(ITxEvent.Type.BEGIN, tx))
					putEvent(new TxEvent(ITxEvent.Type.END, tx))
				}
			}
			def rowendtime = [0]
			events.keySet().each{long time ->
				def value=events.get(time)
				def starts=value.findAll{ITxEvent event ->event.type==ITxEvent.Type.BEGIN}
				starts.each {ITxEvent event ->
					Tx tx = event.transaction
					def rowIdx = 0
					for(rowIdx=0; rowIdx<rowendtime.size() && rowendtime[rowIdx]>tx.beginTime; rowIdx++);
					if(rowendtime.size<=rowIdx)
						rowendtime<<tx.endTime
					else
						rowendtime[rowIdx]=tx.endTime
					tx.concurrencyIndex=rowIdx
				}
			}
			maxConcurrency=rowendtime.size()
		}
		return maxConcurrency
	}

	private putEvent(ITxEvent event){
		if(!events.containsKey(event.time)) 
			events.put(event.time, [event])
		else
			events[event.time]<<event
	}
	
	@Override
	public NavigableMap getEvents() {
		return events;
	}

	@Override
	public Collection getWaveformEventsAtTime(Long time) {
		return events.get(time);
	}
	
	@Override
	public Boolean equals(IWaveform<? extends IWaveformEvent> other) {
		return(other instanceof TxStream && this.getId()==other.getId());
	}

}
