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
package com.minres.scviewer.database;

public class EventTime implements Comparable<EventTime>{
	public enum Unit {
		FS("fs"), PS("ps"), NS("ns"), US("us"), MS("ms"), SEC("s");
		
		private String alternative;
		private Unit(String alternative){
			this.alternative=alternative;
		}
		
		public static Unit fromString(String text) {
			if (text != null)
				for (Unit b : Unit.values()) {
					if (text.equalsIgnoreCase(b.name()) || text.equalsIgnoreCase(b.alternative)) return b;
				}
			return null;
		}
	}
	
	static final double[] scales = {1,1000.0,1000000.0,1000000000.0,1000000000000.0};

	public static final EventTime ZERO = new EventTime(0L);
	
	private long value; // unit is femto seconds
	
	public EventTime(Long value){
		this(value, Unit.FS);
	}
	
	public EventTime(Long value, Unit scale){
		setValue(value, scale);
	}
	
	public static double getScalingFactor(Unit scale){
		return scales[scale.ordinal()];
	}
	
	public long getValue(){
		return(value);
	}
	
	public double getScaledValue(Unit scale){
		return value/scales[scale.ordinal()];
	}
	
	public void setValue(long value){
		this.value=value;
	}
	
	public void setValue(long value, Unit scale){
		this.value=(long) (value*scales[scale.ordinal()]);		
	}
	
	public String toString(){
		return value/scales[Unit.NS.ordinal()] +"ns";
	}

	@Override
	public int compareTo(EventTime other) {
		return this.value<other.value? -1 : this.value==other.value? 0 : 1;
	}
}
