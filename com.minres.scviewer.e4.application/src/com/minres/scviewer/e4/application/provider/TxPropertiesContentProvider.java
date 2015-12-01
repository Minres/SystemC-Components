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

import org.eclipse.jface.viewers.IStructuredContentProvider;
import org.eclipse.jface.viewers.Viewer;

import com.google.common.base.Predicate;
import com.google.common.collect.Collections2;
import com.minres.scviewer.database.IHierNode;
import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.ITxAttribute;

/**
 * The Class TxPropertiesContentProvider. Not used atm
 */
public class TxPropertiesContentProvider implements IStructuredContentProvider {

	/** The show nodes. */
	//	private List<HierNode> nodes;
	private boolean showNodes=false;

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
		showNodes=!(newInput instanceof IHierNode);
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.viewers.IStructuredContentProvider#getElements(java.lang.Object)
	 */
	@Override
	public Object[] getElements(Object inputElement) {
		if(inputElement instanceof ITx){
			return Collections2.filter(((ITx)inputElement).getAttributes(), new Predicate<ITxAttribute>(){
				@Override
				public boolean apply(ITxAttribute arg0) {
					return (arg0 instanceof ITx)!=showNodes;
				}
			}).toArray();
		}else if(inputElement instanceof List<?>)
			return ((List<?>)inputElement).toArray();
		else
			return null;
	}

}
