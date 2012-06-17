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
package com.itjw.txviewer.graph.views.provider;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.jface.viewers.ILabelProvider;
import org.eclipse.jface.viewers.ILabelProviderListener;
import org.eclipse.swt.graphics.Image;

import com.itjw.txviewer.database.ITrDb;
import com.itjw.txviewer.database.ITrHierNode;
import com.itjw.txviewer.database.ITrStream;
import com.itjw.txviewer.graph.TxEditorPlugin;

public class TxDbTreeLabelProvider implements ILabelProvider {

	private List<ILabelProviderListener> listeners = new ArrayList<ILabelProviderListener>();

	private Image database;
	private Image stream;
	private Image folder;
	
	
	public TxDbTreeLabelProvider() {
		super();
		database=TxEditorPlugin.createImage("database");
		stream=TxEditorPlugin.createImage("stream");
		folder=TxEditorPlugin.createImage("folder");
	}

	@Override
	public void addListener(ILabelProviderListener listener) {
		  listeners.add(listener);
	}

	@Override
	public void dispose() {
		if(database!=null) database.dispose();
		if(stream!=null) stream.dispose();
		if(folder!=null) folder.dispose();
	}

	@Override
	public boolean isLabelProperty(Object element, String property) {
		  return false;
	}

	@Override
	public void removeListener(ILabelProviderListener listener) {
		  listeners.remove(listener);
	}

	@Override
	public Image getImage(Object element) {
		if(element instanceof ITrDb){
			return database;
		}else if(element instanceof ITrStream){
			return stream;
		}else if(element instanceof ITrHierNode){
			return folder;
		} else
			return null;
	}

	@Override
	public String getText(Object element) {
		return ((ITrHierNode)element).getName();
	}

}


