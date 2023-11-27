import numpy
import torch
import torch.nn as nn
import torch.nn.functional as F
import re
import matplotlib.pyplot as plt
from typing import List


class char_tokenizer:
    """
    a very simple char-based tokenizer. the tokenizer turns a string into a list of integers.
    """

    def __init__(self, corpus: List[str]):
        if len(corpus) == 0:
            return
        self.vocab_dic = {char: index for index, char in enumerate(set(corpus))}
        self.index_dic = {index: char for char, index in self.vocab_dic.items()}
        self.n_vocab = len(self.vocab_dic)

    def encode(self, string):
        return [self.vocab_dic[char] for char in string]

    def decode(self, codes: List[int]):
        return ''.join([self.index_dic[index] for index in codes])

    def save(self, path):
        torch.save(self.vocab_dic, path)

    def load(self, path):
        self.vocab_dic = torch.load(path)
        self.index_dic = {index: char for char, index in self.vocab_dic.items()}
        self.n_vocab = len(self.vocab_dic)


class Head(nn.Module):
    """single head of self-attention"""

    def __init__(self, head_size):
        super().__init__()

        self.Key = nn.Linear(n_embd, head_size)
        self.Query = nn.Linear(n_embd, head_size)
        self.Value = nn.Linear(n_embd, head_size)

        self.register_buffer("tril", torch.tril(torch.ones(block_size, block_size)))

    def forward(self, inputs):
        keys = self.Key(inputs)
        queries = self.Query(inputs)
        values = self.Value(inputs)

        attention = torch.matmul(queries, keys.transpose(-2, -1)) / numpy.sqrt(keys.size(-1))
        attention = attention.masked_fill(
            self.tril[:attention.size(-2), :attention.size(-1)] == 0, -torch.inf
        )
        attention = F.softmax(attention, dim=-1)
        attention = torch.matmul(attention, values)

        return attention


class MultiHeadAttention(nn.Module):
    def __init__(self, n_heads, head_size):
        super().__init__()

        self.n_heads = n_heads
        self.head_size = head_size
        self.heads = nn.ModuleList([
            Head(head_size) for _ in range(n_heads)
        ])
        self.projection = nn.Linear(n_heads * head_size, n_embd)

    def forward(self, inputs):
        out = torch.cat([head(inputs) for head in self.heads], dim=-1)
        return self.projection(out)


class FeedForward(nn.Module):
    def __init__(self, n_embd):
        super().__init__()

        self.net = nn.Sequential(
            nn.Linear(n_embd, 32 * n_embd),
            nn.ReLU(),
            nn.Linear(32 * n_embd, n_embd)
        )

    def forward(self, inputs):
        return self.net(inputs)


class Block(nn.Module):
    def __init__(self, n_embd, n_heads):
        super().__init__()

        self.attention = MultiHeadAttention(n_heads, n_embd // n_heads)
        self.attention_norm = nn.LayerNorm(n_embd)
        self.feedforward = FeedForward(n_embd)
        self.feedforward_norm = nn.LayerNorm(n_embd)

    def forward(self, inputs):
        attention_out = self.attention_norm(inputs + self.attention(inputs))
        feedforward_out = self.feedforward_norm(attention_out + self.feedforward(attention_out))
        return feedforward_out


class PositionalEncoding(nn.Module):
    def __init__(self, n_embd):
        super().__init__()
        self.n_embd = n_embd
        self.encoding = torch.zeros(block_size, n_embd, device=device)
        self.encoding.requires_grad = False
        pos = torch.arange(0, block_size, dtype=torch.float, device=device).unsqueeze(1)
        div_term = torch.exp(torch.arange(0, n_embd, 2, dtype=torch.float, device=device)) / (-n_embd * numpy.log(1e4))

        self.encoding[:, 0::2] = torch.sin(pos * div_term)
        self.encoding[:, 1::2] = torch.cos(pos * div_term)

    def forward(self, inputs):
        _, length = inputs.shape
        return self.encoding[:length, :]


class Transformer(nn.Module):
    def __init__(self):
        super().__init__()
        self.embedding = nn.Embedding(n_vocab, n_embd)
        self.positional_encoding = PositionalEncoding(n_embd)
        self.blocks = nn.ModuleList([
            Block(n_embd, n_heads) for _ in range(n_layers)
        ])
        self.norm = nn.LayerNorm(n_embd)
        self.linear = nn.Linear(n_embd, n_vocab)

    def forward(self, inputs, labels=None):
        embedding = self.embedding(inputs)
        positional_encoding = self.positional_encoding(inputs)
        out = embedding + positional_encoding

        for block in self.blocks:
            out = block(out)

        logits = self.linear(out)

        # compute the loss

        if labels is None:
            loss = None
        else:
            batch, time, channel = logits.shape
            logits = logits.view(batch * time, channel)
            labels = labels.view(batch * time)
            loss = F.cross_entropy(logits, labels)
        return logits, loss

    def generate(self, inputs, max_new_tokens):
        for _ in range(max_new_tokens):
            logits, loss = self.forward(inputs[:, -block_size:])
            probabilities = F.softmax(logits[:, -1, :], dim=-1)
            sampled_token = torch.multinomial(probabilities, num_samples=1)
            inputs = torch.cat([inputs, sampled_token], dim=1)

        return inputs


def get_batch(split):
    data = train_data if split == "train" else val_data
    ix = torch.randint(len(data) - block_size, (batch_size,))
    x = torch.stack([data[i: i + block_size] for i in ix])
    y = torch.stack([data[i + 1: i + block_size + 1] for i in ix])
    x, y = x.to(device), y.to(device)
    return x, y


@torch.no_grad()
def estimate_loss(model):
    out = {}
    model.eval()
    for split in ["train", "val"]:
        losses = torch.zeros(eval_iters)
        for k in range(eval_iters):
            x, y = get_batch(split)
            logits, loss = model(x, y)
            losses[k] = loss.item()
        out[split] = losses.mean()
    return out


def generate(model, text=""):
    if len(text) == 0:
        context = torch.zeros((1, 1), device=device, dtype=torch.long)
    else:
        context = torch.tensor([tokenizer.encode(text)], device=device, dtype=torch.long)
    print(decode(model.generate(context, max_new_tokens=500)[0].tolist()))


def draw_losses_curve(train_losses, val_losses):
    plt.plot(range(len(train_losses)), train_losses, label='Train Loss')
    plt.plot(range(len(val_losses)), val_losses, label='Val Loss')
    plt.xlabel(f'Iter /{eval_interval}')
    plt.ylabel('Loss')
    plt.legend()
    plt.show()
    plt.savefig('../losses.png')


def train(model):
    optimizer = torch.optim.AdamW(model.parameters(), lr=learning_rate)

    train_losses = []
    val_losses = []
    for iter in range(max_iters):

        if iter % eval_interval == 0:
            losses = estimate_loss(model)
            print(
                f"step {iter}: train loss {losses['train']:.4f}, val loss {losses['val']:.4f}"
            )
            train_losses.append(losses['train'])
            val_losses.append(losses['val'])

        inputs, labels = get_batch("train")

        logits, loss = model(inputs, labels)
        optimizer.zero_grad(set_to_none=True)
        loss.backward()
        optimizer.step()

    draw_losses_curve(train_losses, val_losses)


def text_split(string):
    pattern = r'[a-zA-Z]+|[.,!?;:"\'\s\n]'
    words = re.findall(pattern, string)
    return words


# define the hyperparameters
batch_size = 32
block_size = 256
max_iters = 12001  # set the number of training iterations as you like
eval_interval = 100
learning_rate = 1e-3
device = "cuda" if torch.cuda.is_available() else "cpu"
eval_iters = 200
n_embd = 64
n_heads = 8
n_layers = 6
torch.manual_seed(3407)

is_train = False

if is_train:
    # read the dataset
    with open("../data/input.txt", "r", encoding="utf-8") as f:
        text = f.read()
    chars = sorted(list(set(text)))

    # initialize the vocabulary
    tokenizer = char_tokenizer(chars)
    encode = tokenizer.encode
    decode = tokenizer.decode
    n_vocab = tokenizer.n_vocab

    # separate the dataset into train and validation
    train_data = torch.tensor(encode(text[: -len(text) // 10]), dtype=torch.long)
    val_data = torch.tensor(encode(text[-len(text) // 10:]), dtype=torch.long)

    # define the model
    model = Transformer().to(device)
    train(model)
    tokenizer.save("../model/vocab.pt")
    torch.save(model.state_dict(), "../model/model.pt")
else:
    tokenizer = char_tokenizer([])
    tokenizer.load("../model/vocab.pt")
    encode = tokenizer.encode
    decode = tokenizer.decode
    n_vocab = tokenizer.n_vocab
    model = Transformer().to(device)
    model.load_state_dict(torch.load("../model/model.pt", map_location=device))

generate(model, "To be, or not to be: that is the")
