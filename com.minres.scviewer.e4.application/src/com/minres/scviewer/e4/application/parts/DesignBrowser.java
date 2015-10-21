package com.minres.scviewer.e4.application.parts;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.Arrays;
import java.util.List;

import javax.annotation.PostConstruct;
import javax.inject.Inject;

import org.eclipse.e4.core.di.annotations.Optional;
import org.eclipse.e4.core.services.events.IEventBroker;
import org.eclipse.e4.ui.di.Focus;
import org.eclipse.e4.ui.di.UIEventTopic;
import org.eclipse.e4.ui.workbench.modeling.ESelectionService;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.ISelectionChangedListener;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.jface.viewers.TreeViewer;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;

import com.minres.scviewer.database.IWaveformDb;
import com.minres.scviewer.e4.application.provider.TxDbContentProvider;
import com.minres.scviewer.e4.application.provider.TxDbLabelProvider;

public class DesignBrowser implements ISelectionChangedListener {

	@Inject IEventBroker eventBroker;
	
	@Inject	ESelectionService selectionService;

	private TreeViewer contentOutlineViewer;

	private PropertyChangeListener l = new PropertyChangeListener() {
		@Override
		public void propertyChange(PropertyChangeEvent evt) {
			if("CHILDS".equals(evt.getPropertyName())){
				contentOutlineViewer.getTree().getDisplay().asyncExec(new Runnable() {					
					@Override
					public void run() {
						contentOutlineViewer.refresh();
					}
				});
			}
		}
	};
	
	@PostConstruct
	public void createComposite(Composite parent) {
		parent.setLayout(new GridLayout(1, false));
		contentOutlineViewer = new TreeViewer(parent, SWT.MULTI | SWT.H_SCROLL | SWT.V_SCROLL);
		contentOutlineViewer.addSelectionChangedListener(this);
		contentOutlineViewer.getTree().setLayoutData(new GridData(GridData.FILL_BOTH));
		contentOutlineViewer.setContentProvider(new TxDbContentProvider());
		contentOutlineViewer.setLabelProvider(new TxDbLabelProvider());
		contentOutlineViewer.setUseHashlookup(true);
	}

	@Focus
	public void setFocus() {
		contentOutlineViewer.getTree().setFocus();
		setSelection(contentOutlineViewer.getSelection());
	}

	@Override
	public void selectionChanged(SelectionChangedEvent event) {
		setSelection(event.getSelection());
	}

	protected void setSelection(ISelection iSelection) {
		IStructuredSelection selection = (IStructuredSelection)iSelection;
		switch(selection.size()){
		case 0:
			eventBroker.post(WaveformViewerPart.ACTIVE_NODE, null);
			break;
		case 1:
			eventBroker.post(WaveformViewerPart.ACTIVE_NODE, selection.getFirstElement());
			selectionService.setSelection(selection.getFirstElement());
			break;
		default:
			eventBroker.post(WaveformViewerPart.ACTIVE_NODE, selection.getFirstElement());
			selectionService.setSelection(selection.toList());
			break;
		}
	}

	@SuppressWarnings("unchecked")
	@Inject @Optional
	public void  getStatusEvent(@UIEventTopic(WaveformViewerPart.ACTIVE_DATABASE) IWaveformDb database) {
		Object input = contentOutlineViewer.getInput();
		if(input!=null && input instanceof List<?>)
			((List<IWaveformDb>)input).get(0).removePropertyChangeListener(l);
		contentOutlineViewer.setInput(Arrays.asList(new IWaveformDb[]{database}));
		// Set up the tree viewer
		database.addPropertyChangeListener(l);
	} 
/*
 * 	TODO: needs top be implemented
	@Inject @Optional
	public void  getStatusEvent(@UIEventTopic(WaveformViewerPart.ACTIVE_NODE_PATH) String path) {
		
	}
*/
};