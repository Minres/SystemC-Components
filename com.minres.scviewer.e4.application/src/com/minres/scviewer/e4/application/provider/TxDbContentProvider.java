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

import java.util.Collection;
import java.util.List;

import org.eclipse.jface.viewers.ITreeContentProvider;
import org.eclipse.jface.viewers.Viewer;

import com.google.common.base.Predicate;
import com.google.common.collect.Collections2;
import com.minres.scviewer.database.IHierNode;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformDb;

/**
 * The Class TxDbContentProvider providing the tree content of a database for the respective viewer.
 */
public class TxDbContentProvider implements ITreeContentProvider {

	/** The show nodes. */
	//	private List<HierNode> nodes;
	private boolean showNodes;

	/**
	 * Instantiates a new tx db content provider.
	 */
	public TxDbContentProvider() {
		super();
		this.showNodes = false;
	}

	/**
	 * Instantiates a new tx db content provider.
	 *
	 * @param showNodes the show nodes
	 */
	public TxDbContentProvider(boolean showNodes) {
		super();
		this.showNodes = showNodes;
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.viewers.IContentProvider#dispose()
	 */
	@Override
	public void dispose() {	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.viewers.IContentProvider#inputChanged(org.eclipse.jface.viewers.Viewer, java.lang.Object, java.lang.Object)
	 */
	@Override
	public void inputChanged(Viewer viewer, Object oldInput, Object newInput) {
//		showNodes=!(newInput instanceof IHierNode);
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.viewers.ITreeContentProvider#getElements(java.lang.Object)
	 */
	@Override
	public Object[] getElements(Object inputElement) {
		if(inputElement instanceof IHierNode){
			Collection<IHierNode> res = Collections2.filter(((IHierNode)inputElement).getChildNodes(), new Predicate<IHierNode>(){
				@Override
				public boolean apply(IHierNode arg0) {
					if(showNodes){
						return arg0 instanceof IWaveform<?>;
					} else{
						return arg0.getChildNodes().size()!=0;
					}
				}
			});
			return res.toArray();
		}else if(inputElement instanceof List<?>){
			return ((List<?>)inputElement).toArray();
		}else if(inputElement instanceof IWaveformDb){
			return new Object[]{};
		} else
			return null;
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.viewers.ITreeContentProvider#getChildren(java.lang.Object)
	 */
	@Override
	public Object[] getChildren(Object parentElement) {
		return getElements(parentElement);
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.viewers.ITreeContentProvider#getParent(java.lang.Object)
	 */
	@Override
	public Object getParent(Object element) {
		return null;
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.viewers.ITreeContentProvider#hasChildren(java.lang.Object)
	 */
	@Override
	public boolean hasChildren(Object element) {
		Object[] obj = getElements(element);
		return obj == null ? false : obj.length > 0;
	}

}
