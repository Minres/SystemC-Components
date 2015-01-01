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
package com.itjw.txviewer.ui;

import java.util.List;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.jface.action.Action;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Event;
import org.eclipse.ui.IActionBars;
import org.eclipse.ui.IEditorInput;
import org.eclipse.ui.IEditorSite;
import org.eclipse.ui.IFileEditorInput;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.ide.FileStoreEditorInput;
import org.eclipse.ui.part.EditorPart;
import org.eclipse.ui.views.contentoutline.IContentOutlinePage;
import org.eclipse.ui.views.properties.IPropertySheetPage;
import org.eclipse.ui.views.properties.tabbed.ITabbedPropertySheetPageContributor;
import org.eclipse.ui.views.properties.tabbed.TabbedPropertySheetPage;
import org.osgi.framework.BundleContext;
import org.osgi.framework.FrameworkUtil;
import org.osgi.framework.ServiceReference;

import com.itjw.txviewer.database.ITrDb;
import com.itjw.txviewer.database.ITrStream;
import com.itjw.txviewer.database.ITransactionDbFactory;
import com.itjw.txviewer.ui.swt.TxDisplay;
import com.itjw.txviewer.ui.views.TxOutlinePage;

public class TxEditorPart extends EditorPart implements ITabbedPropertySheetPageContributor {

	public static final String ID = "com.itjw.txviewer.ui.TxEditorPart"; //$NON-NLS-1$

	public static final String WAVE_ACTION_ID = "com.itjw.txviewer.ui.action.AddToWave";

	private TxDisplay txDisplay;
	
	/** This is the root of the editor's model. */
	private ITrDb database;

	private Composite myParent;

	public TxEditorPart() {
	}

	/**
	 * Create contents of the editor part.
	 * @param parent
	 */
	@Override
	public void createPartControl(Composite parent) {
		myParent=parent;
		/** Add handlers for global actions (delete, etc) */
		IActionBars actionBars = getEditorSite().getActionBars();
		actionBars.setGlobalActionHandler(WAVE_ACTION_ID,	new Action() {
			@Override
			public void runWithEvent(Event event) {
				System.out.println("AddToWave with event");
			}
			
			@Override
			public void run() {
				System.out.println("AddToWave");
			}
		});
		
		txDisplay = new TxDisplay(parent);
		getSite().setSelectionProvider(txDisplay);
		if(getEditorInput()!=null && ((TxEditorInput) getEditorInput()).getStreamNames().size()>0){
			if(MessageDialog.openConfirm(parent.getShell(), "Confirm", "Do you want the restore last state of the wave form?"))
				for(String streamName:((TxEditorInput) getEditorInput()).getStreamNames()){
					ITrStream stream = database.getStreamByName(streamName);
					if(stream!=null)
						txDisplay.addStream(stream);
				}
		}
	}

	
	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.ui.part.EditorPart#setInput(org.eclipse.ui.IEditorInput)
	 */
	protected void setInput(IEditorInput input) {
		super.setInput(input);
		if(input instanceof IFileEditorInput){
			if(!(input instanceof TxEditorInput))
				super.setInput(new TxEditorInput(((IFileEditorInput)input).getFile()));
			try {
				IFile file = ((IFileEditorInput) input).getFile();
				getTrDatabase();
				database.load(file.getContents());
				setPartName(((IFileEditorInput) input).getFile().getName());
			} catch (Exception e) {
				handleLoadException(e);
			}
		} else if(input instanceof FileStoreEditorInput){
			try {
				getTrDatabase();
				database.load(((FileStoreEditorInput) input).getURI().toURL().openStream());
				setPartName(((FileStoreEditorInput) input).getName());
			} catch (Exception e) {
				handleLoadException(e);
			}
		}
	}

	protected void getTrDatabase() {
		BundleContext context = FrameworkUtil.getBundle(this.getClass()).getBundleContext();
		ServiceReference<?> serviceReference = context.getServiceReference(ITransactionDbFactory.class.getName());
		database = ((ITransactionDbFactory) context.getService(serviceReference)).createDatabase();
		if(txDisplay !=null) database.addPropertyChangeListener(txDisplay);
	}

	private void handleLoadException(Exception e) {
		System.err.println("** Load failed. Using default model. **");
		e.printStackTrace();
		database = null;
	}

	@Override
	public void setFocus() {
		myParent.setFocus();
	}

	@Override
	public void doSave(IProgressMonitor monitor) {
	}

	@Override
	public void doSaveAs() {
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

	public ITrDb getDatabase() {
		return database;
	}

	public void addStreamToList(ITrStream stream){
		if(getEditorInput() instanceof TxEditorInput && !((TxEditorInput) getEditorInput()).getStreamNames().contains(stream.getFullName())){
			((TxEditorInput) getEditorInput()).getStreamNames().add(stream.getFullName());
			txDisplay.addStream(stream);
		} else
			txDisplay.addStream(stream);
			
	}
	
	public void addStreamsToList(ITrStream[] streams){
		for(ITrStream stream:streams)
			addStreamToList(stream);
	}

	public void removeStreamFromList(ITrStream stream){
		if(getEditorInput() instanceof TxEditorInput && ((TxEditorInput) getEditorInput()).getStreamNames().contains(stream.getFullName())){
			((TxEditorInput) getEditorInput()).getStreamNames().remove(stream.getFullName());
			txDisplay.removeStream(stream);
		} else
			txDisplay.removeStream(stream);
	}
	
	public void removeStreamsFromList(ITrStream[] streams){
		for(ITrStream stream:streams)
			removeStreamFromList(stream);
	}
	
	public List<ITrStream> getStreamList(){
		return txDisplay.getStreamList();
	}

	public void setSelection(final ISelection selection){
		myParent.getDisplay().asyncExec(new Runnable() {
			@Override
			public void run() {
				txDisplay.setSelection(selection);
			}
		});
	}
	
	@Override
	public String getContributorId() {
		return getSite().getId();
	}

}
