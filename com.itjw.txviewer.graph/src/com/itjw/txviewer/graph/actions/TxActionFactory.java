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
package com.itjw.txviewer.graph.actions;

import java.util.LinkedList;

import org.eclipse.jface.action.Action;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.ui.ISharedImages;
import org.eclipse.ui.PlatformUI;

import com.itjw.txviewer.database.ITrHierNode;
import com.itjw.txviewer.database.ITrStream;
import com.itjw.txviewer.database.ITransaction;
import com.itjw.txviewer.graph.TransactionSelection;
import com.itjw.txviewer.graph.TxEditorPlugin;
import com.itjw.txviewer.graph.data.ITransactionFacade;

public class TxActionFactory {
	public static final int ADD_TO_WAVE = 0;
	public static final int ADD_ALL_TO_WAVE = 1;
	public static final int REMOVE_FROM_WAVE = 2;
	public static final int REMOVE_ALL_FROM_WAVE = 3;
	public static final int JUMP_TO_TX = 4;
	private static TxActionFactory instance;

	private TxActionFactory(){}
	
	public static TxActionFactory getInstance() {
		if(instance == null) instance=new TxActionFactory();
		return instance;
	}

	public static Action getAction(int actionId, boolean enabled){
		switch(actionId){
		case ADD_TO_WAVE:
			return getInstance().makeStreamAction("Add to Wave", ISharedImages.IMG_OBJ_ADD, enabled, false);
		case ADD_ALL_TO_WAVE:
			return getInstance().makeStreamAction("Add all to Wave", ISharedImages.IMG_OBJ_ADD, true, false);
		case REMOVE_FROM_WAVE:
			return getInstance().makeStreamAction("Remove from Wave", ISharedImages.IMG_TOOL_DELETE, enabled, true);
		case REMOVE_ALL_FROM_WAVE:
			return getInstance().makeStreamAction("Remove all from Wave", ISharedImages.IMG_TOOL_DELETE, true, true);
		case JUMP_TO_TX:
			return getInstance().makeTransactionAction("Jump to Transaction", ISharedImages.IMG_OBJ_ADD, true);
		}
		return null;
	}
	
	private Action makeStreamAction(String text, String imgDescriptor, boolean enabled, final boolean remove) {
		Action action = new Action() {
			public void run() {
				ISelection selection = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getSelectionService().getSelection();
				for(Object obj :((IStructuredSelection) selection).toArray()){
					if(obj instanceof ITrStream){
						if(remove)
							TxEditorPlugin.getDefault().getOpenEditorPart().removeStreamFromList((ITrStream) obj);
						else
							TxEditorPlugin.getDefault().getOpenEditorPart().addStreamToList((ITrStream) obj);
					} else if(obj instanceof ITrHierNode){
						LinkedList<ITrHierNode> queue = new LinkedList<ITrHierNode>();
						LinkedList<ITrStream> streams = new LinkedList<ITrStream>();
						queue.add((ITrHierNode)obj);
						while(queue.size()>0){
							ITrHierNode n = queue.poll();
							if(n instanceof ITrStream) streams.add((ITrStream) n);
							queue.addAll(n.getChildNodes());
						}
						if(remove)
							TxEditorPlugin.getDefault().getOpenEditorPart().removeStreamsFromList(streams.toArray(new ITrStream[]{}));
						else
							TxEditorPlugin.getDefault().getOpenEditorPart().addStreamsToList(streams.toArray(new ITrStream[]{}));
					}
				}
			}
		};
		action.setText(text);
		action.setImageDescriptor(PlatformUI.getWorkbench().getSharedImages().getImageDescriptor(imgDescriptor));
		action.setEnabled(enabled);
		return action;
	}

	private Action makeTransactionAction(String text, String imgDescriptor, boolean enabled) {
		Action action = new Action() {
			public void run() {
				ISelection selection = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getSelectionService().getSelection();
				for(Object obj :((IStructuredSelection) selection).toArray()){
					ISelection sel=null;
					if(obj instanceof ITransactionFacade){
						sel = new TransactionSelection((ITransactionFacade) obj);
					} else if(obj instanceof ITransaction){
						sel = new TransactionSelection(new ITransactionFacade( (ITransaction) obj));
					}
					if(sel!=null)
						TxEditorPlugin.getDefault().getOpenEditorPart().setSelection(sel);
				}
			}
		};
		action.setText(text);
		action.setImageDescriptor(PlatformUI.getWorkbench().getSharedImages().getImageDescriptor(imgDescriptor));
		action.setEnabled(enabled);
		return action;
	}
}
