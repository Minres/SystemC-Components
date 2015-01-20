package com.minres.scviewer.database;

import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import java.util.ArrayList;
import java.util.List;

public class HierNode implements IHierNode {

	protected String name;
	
	protected ArrayList<IHierNode> childs;
	
	protected PropertyChangeSupport pcs;

	public HierNode() {
		childs = new ArrayList<IHierNode>();
		pcs=new PropertyChangeSupport(this);
	}

	public HierNode(String name) {
		this();
		this.name=name;
	}

	@Override
	public void addPropertyChangeListener(PropertyChangeListener l) {
		pcs.addPropertyChangeListener(l);
	}

	@Override
	public void removePropertyChangeListener(PropertyChangeListener l) {
		pcs.removePropertyChangeListener(l);
	}


	@Override
	public String getFullName() {
		return name;
	}

	@Override
	public String getName() {
		return name;
	}

	@Override
	public void setName(String name) {
		this.name=name;
	}

	@Override
	public List<IHierNode> getChildNodes() {
		return childs;
	}

	@Override
	public int compareTo(IHierNode o) {
		return name.compareTo(o.getName());
	}

}
