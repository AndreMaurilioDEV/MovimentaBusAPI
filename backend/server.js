// backend/server.js
const express = require('express');
const cors = require('cors');
const app = express();


app.use(cors());
app.use(express.json());


let dadosOnibus = [];


app.post('/api/onibus', (req, res) => {
  try {
    const { idOnibus, totalPessoas, entradas, saidas } = req.body;
    
    const novoDado = {
      idOnibus: idOnibus || 1920,
      totalPessoas: totalPessoas || 0,
      entradas: entradas || 0,
      saidas: saidas || 0,
      timestamp: new Date().toLocaleString('pt-BR')
    };
    

    dadosOnibus.push(novoDado);
    

    if (dadosOnibus.length > 100) {
      dadosOnibus = dadosOnibus.slice(-100);
    }
    
    console.log('🚌 Dados recebidos:', novoDado);
    
    res.status(200).json({ 
      success: true, 
      message: 'Dados recebidos com sucesso!',
      data: novoDado
    });
    
  } catch (error) {
    console.error('❌ Erro:', error);
    res.status(500).json({ success: false, error: error.message });
  }
});


app.get('/api/onibus', (req, res) => {
  res.json({
    success: true,
    data: dadosOnibus,
    total: dadosOnibus.length
  });
});


app.get('/api/onibus/ultimo', (req, res) => {
  const ultimo = dadosOnibus[dadosOnibus.length - 1] || {};
  res.json({
    success: true,
    data: ultimo
  });
});


app.get('/', (req, res) => {
  res.send('🚌 API do Sistema de Ônibus - Funcionando!');
});


const PORT = process.env.PORT || 80;
app.listen(PORT, () => {
  console.log(`Servidor rodando na porta ${PORT}`);
  console.log(`Acesse: http://localhost:${PORT}`);
  console.log(`Endpoint: http://localhost:${PORT}/api/onibus`);
});