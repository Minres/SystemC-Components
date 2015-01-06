package com.minres.scviewer.database;

import java.beans.PropertyChangeListener;
import java.util.ArrayList;
import java.util.List;

public class HierNode implements IHierNode {

	protected String name;
	
	protected ArrayList<IHierNode> childs;
	
	public HierNode(String name) {
		this.name=name;
		childs = new ArrayList<IHierNode>();
	}

	@Override
	public void addPropertyChangeListener(PropertyChangeListener l) {
	}

	@Override
	public void removePropertyChangeListener(PropertyChangeListener l) {
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

}
