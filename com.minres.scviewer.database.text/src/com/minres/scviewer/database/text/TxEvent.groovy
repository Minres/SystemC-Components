package com.minres.scviewer.database.text;

import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.ITxEvent;
import com.minres.scviewer.database.IWaveformEvent;

class TxEvent implements ITxEvent {

	final ITxEvent.Type type;
	
	Tx transaction;
	
	TxEvent(ITxEvent.Type type, ITx transaction) {
		super();
		this.type = type;
		this.transaction = transaction;
	}

	@Override
	IWaveformEvent duplicate() throws CloneNotSupportedException {
		new TxEvent(type, transaction, time)
	}

	@Override
	int compareTo(IWaveformEvent o) {
		time.compareTo(o.getTime())
	}

	Long getTime(){
		return type==ITxEvent.Type.BEGIN?transaction.beginTime:transaction.endTime
	}
}
