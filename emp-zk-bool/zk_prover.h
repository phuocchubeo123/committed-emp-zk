#ifndef ZK_PROVER_H__
#define ZK_PROVER_H__
#include <emp-tool/emp-tool.h>
#include <emp-ot/emp-ot.h>
#include <iostream>
#include "emp-zk-bool/ostriple.h"
#include "emp-zk-bool/zk_bool_circuit_exec.h"

template<typename IO>
class ZKBoolCircExecPrv:public ZKBoolCircExec<IO> { public:
	using ZKBoolCircExec<IO>::pub_label;
	ZKBoolCircExecPrv() {
		PRG prg(fix_key);
		prg.random_block(pub_label, 2);
		pub_label[0] = pub_label[0] & makeBlock(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFEULL);
		pub_label[1] = pub_label[1] & makeBlock(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFEULL);
		pub_label[1] = pub_label[1] ^ makeBlock(0, 1);
	}
	block not_gate(const block&a) override {
		return a ^ makeBlock(0, 1);
	}
	template<typename T>
	void set_ostriple(OSTriple<T> *ostriple) {
		this->ostriple = ostriple;
	}
};



template<typename IO>
class ZKProver: public ProtocolExecution {
public:
	IO* io = nullptr;
	OSTriple<IO> *ostriple;
	ZKBoolCircExecPrv<IO> *gen;
	ZKProver(IO** ios, int threads, ZKBoolCircExecPrv<IO> *t, bool check_zero = false): ProtocolExecution(ALICE) {
		this->io = ios[0];
		this->gen = t;
		ostriple = new OSTriple<IO>(ALICE, threads, ios, check_zero);
		t->template set_ostriple<IO>(ostriple);
	}
	~ZKProver() {
		delete ostriple;
	}

	/* 
	 * Prover is the receiver in iterative COT
	 * interface: get 1 authenticated bit
	 * authenticated message, last bit as the value
	 * embeded in label
	 */
	void feed(block * label, int party, const bool* b, int length) {
		ostriple->authenticated_bits_input(label, b, length);
	}

	/*
	 * check correctness of triples using cut and choose and bucketing
	 * check correctness of the output
	 */
	void reveal(bool* b, int party, const block * label, int length) {
		if(party == ALICE) {
			for(int i = 0; i < length; ++i)
				b[i] = getLSB(label[i]);
		}
		else if (party == BOB or party == PUBLIC) {
			ostriple->verify_output(b, label, length);
		}
	}
};
#endif //ZK_PROVER_H__
