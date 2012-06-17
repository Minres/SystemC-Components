package com.itjw.txviewer.graph.ui.swt;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;

import org.eclipse.core.runtime.ListenerList;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.ISelectionChangedListener;
import org.eclipse.jface.viewers.ISelectionProvider;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.jface.viewers.StructuredSelection;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.SashForm;
import org.eclipse.swt.events.MouseAdapter;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.ScrollBar;
import org.eclipse.ui.IWorkbenchPartSite;

import com.itjw.txviewer.database.ITrStream;
import com.itjw.txviewer.database.ITransaction;
import com.itjw.txviewer.graph.TrHierNodeSelection;
import com.itjw.txviewer.graph.TransactionSelection;
import com.itjw.txviewer.graph.TxEditorPart;
import com.itjw.txviewer.graph.data.ITrDbFacade;
import com.itjw.txviewer.graph.data.ITrStreamFacade;
import com.itjw.txviewer.graph.data.ITransactionFacade;

public class TxDisplay implements PropertyChangeListener, ISelectionProvider{

	private TxEditorPart txEditor;
	private SashForm sashFormTop;
	private ListPane txNamePane;
	private ListPane txValuePane;
	private WaveImageCanvas waveImageCanvas;

    private ListenerList listeners = new ListenerList();
    private ITrStreamFacade currentStreamSelection;  
    private ITransactionFacade currentSelection;  

    public TxDisplay(Composite parent, TxEditorPart txEditorPart, IWorkbenchPartSite site) {
		txEditor = txEditorPart;
		sashFormTop = new SashForm(parent, SWT.HORIZONTAL+SWT.BORDER);
        sashFormTop.setLayoutData(new GridData(GridData.FILL_BOTH));
        final SashForm sashFormLeft = new SashForm(sashFormTop, SWT.HORIZONTAL);
        txNamePane = new NameListPane(sashFormLeft, this);
        txNamePane.addLabelClickListener(this);
        txValuePane = new ValueListPane(sashFormLeft, this);
        txValuePane.addLabelClickListener(this);
        waveImageCanvas = new WaveImageCanvas(sashFormTop, SWT.DOUBLE_BUFFERED|SWT.H_SCROLL|SWT.V_SCROLL);
        waveImageCanvas.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseUp(MouseEvent e) {
				currentSelection = waveImageCanvas.getTransactionAtPos(new Point(e.x, e.y));
				waveImageCanvas.setCurrentSelection(currentSelection);
				if(currentSelection!=null || currentStreamSelection!=null)
					setSelection(getSelection());
				else
					setSelection(StructuredSelection.EMPTY);
			}
		});
        txNamePane.scroll.getVerticalBar().addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				int y = ((ScrollBar) e.widget).getSelection();
				txValuePane.scrollToY(y);
				waveImageCanvas.scrollToY(y);
			}
        });
        txValuePane.scroll.getVerticalBar().addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				int y = ((ScrollBar) e.widget).getSelection();
				txNamePane.scrollToY(y);
				waveImageCanvas.scrollToY(y);
			}
        });
        waveImageCanvas.getVerticalBar().addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				int y = ((ScrollBar) e.widget).getSelection();
				txNamePane.scrollToY(y);
				txValuePane.scrollToY(y);
			}
        });
 
        sashFormTop.setWeights(new int[] {30, 70});
        sashFormLeft.setWeights(new int[] {75, 25});
        
        if(txEditor.getModel()!= null)
        	streamListChanged();
	}

	public TxEditorPart getTxEditor() {
		return txEditor;
	}

	public void streamListChanged() {
		waveImageCanvas.setWaveformList(getTxEditor().getStreamList());
		txNamePane.streamListChanged();
		txValuePane.streamListChanged();
		sashFormTop.redraw();
	}

	public void addSelectionListener(SelectionListener listener) {
		waveImageCanvas.addSelectionListener(listener);
	}

	@Override
	public void propertyChange(PropertyChangeEvent pce) {
		if(pce.getPropertyName().equals(ITrDbFacade.DATABASE)){
			streamListChanged();
		}else if(pce.getPropertyName().equals(ListPane.SELECTION)){
			currentSelection=null;
			ITrStream str = (ITrStream)pce.getNewValue();
			if(str instanceof ITrStreamFacade)
				currentStreamSelection=(ITrStreamFacade)str;
			else
				currentStreamSelection=new ITrStreamFacade(str);
			if(currentStreamSelection!=null)
				setSelection(getSelection());
			else
				setSelection(StructuredSelection.EMPTY);
		}
		
	}
	
	@Override
	public void addSelectionChangedListener(ISelectionChangedListener listener) {
		listeners.add(listener);		
	}

	@Override
	public ISelection getSelection() {
		if(currentSelection!=null)
			return new TransactionSelection(currentSelection);
		else if(currentStreamSelection!=null)
			return new TrHierNodeSelection(currentStreamSelection);
		else
			return null;
	}

	@Override
	public void removeSelectionChangedListener(ISelectionChangedListener listener) {
		listeners.remove(listener);  
	}

	@Override
	public void setSelection(ISelection selection) {
		if(selection instanceof TransactionSelection){
			ITransaction tr =((TransactionSelection)selection).getFirstElement();
			currentSelection = (tr instanceof ITransactionFacade)?(ITransactionFacade)tr:new ITransactionFacade(tr);
		}
		Object[] list = listeners.getListeners();  
		for (int i = 0; i < list.length; i++) {  
			((ISelectionChangedListener) list[i]).selectionChanged(new SelectionChangedEvent(this, selection));  
		}
		if(waveImageCanvas.getCurrentSelection()!=currentSelection)
			waveImageCanvas.setCurrentSelection(currentSelection);
		waveImageCanvas.getDisplay().asyncExec(new Runnable() {
			@Override
			public void run() {
				waveImageCanvas.redraw();
			}
		});
	}

}
