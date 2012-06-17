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
package com.itjw.txviewer.graph;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import org.eclipse.jface.viewers.IStructuredSelection;

import com.itjw.txviewer.graph.data.ITrHierNodeFacade;

public class TrHierNodeSelection implements IStructuredSelection {

	List<ITrHierNodeFacade> selection = new ArrayList<ITrHierNodeFacade>();
	
	public TrHierNodeSelection(ITrHierNodeFacade node){
		selection.add(node);
	}
	
	public TrHierNodeSelection(List<ITrHierNodeFacade> nodes){
		selection.addAll(nodes);
	}
	
	public void add(ITrHierNodeFacade node){
		selection.add(node);
	}
	
	public void addAll(List<ITrHierNodeFacade> nodes){
		selection.addAll(nodes);
	}
	
	@Override
	public boolean isEmpty() {
		return selection.size()==0;
	}

	@Override
	public Object getFirstElement() {
		return selection.get(0);
	}

	@Override
	public Iterator<ITrHierNodeFacade> iterator() {
		return selection.iterator();
	}

	@Override
	public int size() {
		return selection.size();
	}

	@Override
	public Object[] toArray() {
		// TODO Auto-generated method stub
		return selection.toArray();
	}

	@Override
	public List<ITrHierNodeFacade> toList() {
		return selection;
	}

}
