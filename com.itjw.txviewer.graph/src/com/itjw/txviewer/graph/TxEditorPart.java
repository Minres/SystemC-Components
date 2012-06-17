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

import java.util.LinkedList;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.gef.ui.actions.ActionRegistry;
import org.eclipse.gef.ui.parts.SelectionSynchronizer;
import org.eclipse.jface.action.Action;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Event;
import org.eclipse.ui.IActionBars;
import org.eclipse.ui.IEditorInput;
import org.eclipse.ui.IEditorSite;
import org.eclipse.ui.IFileEditorInput;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.part.EditorPart;
import org.eclipse.ui.views.contentoutline.IContentOutlinePage;
import org.eclipse.ui.views.properties.IPropertySheetPage;
import org.eclipse.ui.views.properties.tabbed.ITabbedPropertySheetPageContributor;
import org.eclipse.ui.views.properties.tabbed.TabbedPropertySheetPage;

import com.itjw.txviewer.database.ITrDb;
import com.itjw.txviewer.database.ITrStream;
import com.itjw.txviewer.database.InputFormatException;
import com.itjw.txviewer.graph.actions.AddToWave;
import com.itjw.txviewer.graph.data.ITrDbFacade;
import com.itjw.txviewer.graph.data.ITrStreamFacade;
import com.itjw.txviewer.graph.ui.swt.TxDisplay;

public class TxEditorPart extends EditorPart implements ITabbedPropertySheetPageContributor {

	public static final String ID = "com.itjw.txviewer.graph.TxEditorPart"; //$NON-NLS-1$

	private TxDisplay txDisplay;

	/** This is the root of the editor's model. */
	private ITrDb database;

	private LinkedList<ITrStreamFacade> streamList;
	
	private SelectionSynchronizer synchronizer;

	private ActionRegistry actionRegistry;

	public TxEditorPart() {
		streamList = new LinkedList<ITrStreamFacade>();
		
	}

	/**
	 * Create contents of the editor part.
	 * @param parent
	 */
	@Override
	public void createPartControl(Composite parent) {
		/** Add handlers for global actions (delete, etc) */
		IActionBars actionBars = getEditorSite().getActionBars();
		actionBars.setGlobalActionHandler(AddToWave.ID,	new Action() {
			@Override
			public void runWithEvent(Event event) {
				System.out.println("AddToWave with event");
			}
			
			@Override
			public void run() {
				System.out.println("AddToWave");
			}
		});
		
		txDisplay = new TxDisplay(parent, this, getSite());
		TxEditorPlugin.getDefault().editorOpened(this);		
		getSite().setSelectionProvider(txDisplay);
	}

	
	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.ui.part.EditorPart#setInput(org.eclipse.ui.IEditorInput)
	 */
	protected void setInput(IEditorInput input) {
		super.setInput(input);
		try {
			IFile file = ((IFileEditorInput) input).getFile();
			database = new ITrDbFacade();
			if(txDisplay !=null) database.addPropertyChangeListener(txDisplay);
			database.load(file.getContents());
			setPartName(((IFileEditorInput) input).getFile().getName());
//			for(ITrStream stream: database.getAllStreams())
//				streamList.add(new TxStreamAdapter(stream));
		} catch (InputFormatException e) {
			handleLoadException(e);
		} catch (CoreException e) {
			e.printStackTrace();
		}
	}

	private void handleLoadException(Exception e) {
		System.err.println("** Load failed. Using default model. **");
		e.printStackTrace();
		database = null;
	}


	@Override
	public void setFocus() {
		// Set the focus
	}

	@Override
	public void doSave(IProgressMonitor monitor) {
		// Do the Save operation
	}

	@Override
	public void doSaveAs() {
		// Do the Save As operation
	}

	@SuppressWarnings("rawtypes")
	@Override
	public Object getAdapter(Class type) {
		if (type == IContentOutlinePage.class) // outline page
			return new TxOutlinePage(this);
		else if (type == IPropertySheetPage.class) // use tabbed property sheet instead of standard one
            return new TabbedPropertySheetPage(this);
		return super.getAdapter(type);
	}

	public SelectionSynchronizer getSelectionSynchronizer() {
		if (synchronizer == null)
			synchronizer = new SelectionSynchronizer();
		return synchronizer;
	}

	public ActionRegistry getActionRegistry() {
		if (actionRegistry == null)
			actionRegistry = new ActionRegistry();
		return actionRegistry;
	}

	public ITrDb getModel() {
		return database;
	}

		

	@Override
	public void init(IEditorSite site, IEditorInput input)
			throws PartInitException {
		// Initialize the editor part
		setSite(site);
		setInput(input);
	}

	@Override
	public boolean isDirty() {
		return false;
	}

	@Override
	public boolean isSaveAsAllowed() {
		return false;
	}

	public LinkedList<ITrStreamFacade> getStreamList() {
		return streamList;
	}

	public ITrDb getDatabase() {
		return database;
	}

	public void addStreamToList(ITrStream stream){
		if(stream instanceof ITrStreamFacade)
			streamList.add((ITrStreamFacade)stream);
		else
			streamList.add(new ITrStreamFacade(stream));
		txDisplay.streamListChanged();
	}
	
	public void addStreamsToList(ITrStream[] streams){
		for(ITrStream stream: streams) addStreamToList(stream);
		txDisplay.streamListChanged();
	}

	public void removeStreamFromList(ITrStream stream){
		txDisplay.streamListChanged();
	}
	
	public void removeStreamsFromList(ITrStream[] streams){
		txDisplay.streamListChanged();
	}
	
	public void setSelection(ISelection selection){
		txDisplay.setSelection(selection);
	}
	
	@Override
	public String getContributorId() {
		return getSite().getId();
	}

}
