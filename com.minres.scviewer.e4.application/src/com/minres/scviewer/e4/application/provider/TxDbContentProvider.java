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
package com.minres.scviewer.e4.application.provider;

import java.util.List;

import org.eclipse.jface.viewers.ITreeContentProvider;
import org.eclipse.jface.viewers.Viewer;

import com.google.common.base.Predicate;
import com.google.common.collect.Collections2;
import com.minres.scviewer.database.IHierNode;
import com.minres.scviewer.database.IWaveform;

public class TxDbContentProvider implements ITreeContentProvider {

	//	private List<HierNode> nodes;
	private boolean showNodes=false;

	@Override
	public void dispose() {	}

	@Override
	public void inputChanged(Viewer viewer, Object oldInput, Object newInput) {
		showNodes=!(newInput instanceof IHierNode);
	}

	@Override
	public Object[] getElements(Object inputElement) {
		if(inputElement instanceof IHierNode){
			return Collections2.filter(((IHierNode)inputElement).getChildNodes(), new Predicate<IHierNode>(){
				@Override
				public boolean apply(IHierNode arg0) {
					return (arg0 instanceof IWaveform<?>)!=showNodes;
				}
			}).toArray();
		}else if(inputElement instanceof List<?>)
			return ((List<?>)inputElement).toArray();
		else
			return null;
	}

	@Override
	public Object[] getChildren(Object parentElement) {
		return getElements(parentElement);
	}

	@Override
	public Object getParent(Object element) {
		return null;
	}

	@Override
	public boolean hasChildren(Object element) {
		//		Object[] obj = getChildren(element);
		Object[] obj = getElements(element);
		return obj == null ? false : obj.length > 0;
	}

}
