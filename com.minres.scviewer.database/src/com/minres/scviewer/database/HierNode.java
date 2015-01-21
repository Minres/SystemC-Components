/*******************************************************************************
 * Copyright (c) 2014, 2015 MINRES Technologies GmbH and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     MINRES Technologies GmbH - initial API and implementation
 *******************************************************************************/
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
